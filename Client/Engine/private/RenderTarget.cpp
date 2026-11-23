#include "RenderTarget.h"
#include "GameInstance.h"

HRESULT CRenderTarget::Initialize(UINT width, UINT height, DXGI_FORMAT format, const FLOAT clearColor[4],
	D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE srvCpu, D3D12_GPU_DESCRIPTOR_HANDLE srvGpu)
{
	m_Width = width;
	m_Height = height;
	m_Format = format;
	for (int i = 0; i < 4; ++i)
		m_ClearColor[i] = clearColor[i];

	m_Rtv = rtv;
	m_SrvCpu = srvCpu;
	m_SrvGpu = srvGpu;

	return Create_Resource_and_Views();
}

HRESULT CRenderTarget::Resize(UINT width, UINT height)
{
	if (width == m_Width && height == m_Height)
		return S_OK;

	// 디스크립터 칸은 그대로 두고 리소스만 갈아 끼운 뒤 같은 칸에 뷰를 다시 만든다
	Safe_Release(m_Resource);
	m_Width = width;
	m_Height = height;
	return Create_Resource_and_Views();
}

HRESULT CRenderTarget::Create_Resource_and_Views()
{
	ID3D12Device* pDevice = CGameInstance::Get_Instance()->Get_Device();

	// 1. 리소스: 기본 힙(GPU 전용), 렌더타겟으로 쓸 수 있게 ALLOW_RENDER_TARGET
	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = m_Width;
	texDesc.Height = m_Height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = m_Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// 최적화 클리어 값. Clear() 에서 같은 색을 써야 드라이버가 빠른 클리어를 쓸 수 있다
	D3D12_CLEAR_VALUE optClear = {};
	optClear.Format = m_Format;
	for (int i = 0; i < 4; ++i)
		optClear.Color[i] = m_ClearColor[i];

	// 초기 상태는 "셰이더가 읽는 상태"로 둔다.
	// 프레임마다 RENDER_TARGET 으로 바꿔 그리고, 다 그리면 PIXEL_SHADER_RESOURCE 로 되돌리는 흐름을 가정
	m_State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	if (FAILED(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		m_State,
		&optClear,
		IID_PPV_ARGS(&m_Resource))))
	{
		MSG_BOX("Failed to Create : RenderTarget Resource");
		return E_FAIL;
	}

	// 2. RTV: 이 리소스에 그릴 때 OMSetRenderTargets 에 넘기는 뷰 (RTV 힙, 셰이더는 안 봄)
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = m_Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	pDevice->CreateRenderTargetView(m_Resource, &rtvDesc, m_Rtv);

	// 3. SRV: 다음 패스(bright pass, 톤매핑)에서 텍스처로 읽을 때 쓰는 뷰 (shader-visible SRV 힙의 엔진 구간)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = m_Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	pDevice->CreateShaderResourceView(m_Resource, &srvDesc, m_SrvCpu);

	return S_OK;
}

void CRenderTarget::Transition(ID3D12GraphicsCommandList* pCommandList, D3D12_RESOURCE_STATES after)
{
	if (m_State == after)
		return;

	pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Resource, m_State, after));
	m_State = after;
}

void CRenderTarget::Clear(ID3D12GraphicsCommandList* pCommandList)
{
	assert(m_State == D3D12_RESOURCE_STATE_RENDER_TARGET);
	pCommandList->ClearRenderTargetView(m_Rtv, m_ClearColor, 0, nullptr);
}

void CRenderTarget::Bind_as_Output(ID3D12GraphicsCommandList* pCommandList)
{
	assert(m_State == D3D12_RESOURCE_STATE_RENDER_TARGET);

	// 뷰포트가 RT 크기와 다르면 일부만 그려지거나 밖으로 잘린다 (절반 해상도 RT 에 화면 크기 뷰포트 → 왼쪽 위 1/4 만 보임)
	D3D12_VIEWPORT viewport = { 0.f, 0.f, static_cast<float>(m_Width), static_cast<float>(m_Height), 0.f, 1.f };
	D3D12_RECT scissor = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };
	pCommandList->RSSetViewports(1, &viewport);
	pCommandList->RSSetScissorRects(1, &scissor);

	pCommandList->OMSetRenderTargets(1, &m_Rtv, FALSE, nullptr);
}

CRenderTarget* CRenderTarget::Create(UINT width, UINT height, DXGI_FORMAT format, const FLOAT clearColor[4],
	D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE srvCpu, D3D12_GPU_DESCRIPTOR_HANDLE srvGpu)
{
	CRenderTarget* pInstance = new CRenderTarget();
	if (FAILED(pInstance->Initialize(width, height, format, clearColor, rtv, srvCpu, srvGpu)))
	{
		MSG_BOX("Failed to Created : CRenderTarget");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CRenderTarget::Free()
{
	Safe_Release(m_Resource);
}
