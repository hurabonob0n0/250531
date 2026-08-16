#include "ShadowMap.h"
#include "Engine_Config.h"
#include "GameInstance.h"

IMPLEMENT_SINGLETON(CShadowMap)

CShadowMap::CShadowMap()
{
}

void CShadowMap::Initialize()
{
	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	m_Device = pGameInstance->Get_Device();

	mhGpuSrv = pGameInstance->Get_SRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	
	mhCpuDsv = CD3DX12_CPU_DESCRIPTOR_HANDLE(pGameInstance->DepthStencilView(), pGameInstance->Get_DSVSize());

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = SHADOWMAP_SIZE;
	texDesc.Height = SHADOWMAP_SIZE;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_Resource));

	pGameInstance->Add_ShadowMap(&m_Resource);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	m_Device->CreateDepthStencilView(m_Resource, &dsvDesc, mhCpuDsv);

	mViewport = { 0.0f, 0.0f, (float)SHADOWMAP_SIZE, (float)SHADOWMAP_SIZE, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)SHADOWMAP_SIZE, (int)SHADOWMAP_SIZE };


}

void CShadowMap::Late_Update()
{
#pragma region Light青纺贸府

	//2896.31f

	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	m_TankTransform = (CTransform*)pGameInstance->Get_Object_Component("Tank", myTankIndex, "TransformCom");

	XMVECTOR lightDir = XMVectorSet(0.57735f, -0.57735f, 0.57735f, 0.f);
	XMVECTOR lightPos = -2.0f * 300.f* lightDir;
	XMVECTOR targetPos = m_TankTransform->Get_State(CTransform::STATE_POSITION);//XMVectorSet(0.f, 0.f, 0.f, 1.f);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	//XMStoreFloat3(&mLightPosW, lightPos);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - 300.f;
	float b = sphereCenterLS.y - 300.f;
	float n = sphereCenterLS.z - 300.f;
	float r = sphereCenterLS.x + 300.f;
	float t = sphereCenterLS.y + 300.f;
	float f = sphereCenterLS.z + 300.f;

	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	S = lightView * lightProj * T;

	/*XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);*/

	XMMATRIX viewProj = XMMatrixMultiply(lightView, lightProj);

	/* The terrain culls against the light frustum during the shadow pass. */
	pGameInstance->Set_ShadowViewProj(viewProj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(lightView), lightView);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(lightProj), lightProj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	UINT w = SHADOWMAP_SIZE;
	UINT h = SHADOWMAP_SIZE;

	PassConstants pc;

	XMStoreFloat4x4(&pc.View, XMMatrixTranspose(lightView));
	XMStoreFloat4x4(&pc.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&pc.Proj, XMMatrixTranspose(lightProj));
	XMStoreFloat4x4(&pc.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&pc.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&pc.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat3(&pc.EyePosW, lightPos);
	pc.RenderTargetSize = XMFLOAT2((float)w, (float)h);
	pc.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
	pc.NearZ = n;
	pc.FarZ = f;

	auto currPassCB = CGameInstance::Get_Instance()->Get_Current_FrameResource()->m_PassCB;
	currPassCB->CopyData(1, pc);

#pragma endregion Light青纺贸府
}

void CShadowMap::Free()
{
	
}