//#include "TextureMgr.h"
//#include "GameInstance.h"
//
//IMPLEMENT_SINGLETON(CTextureMgr);
//
//int CTextureMgr::Add_Texture(string texname, CTexture* texInstance, TEXTURETYPE TT) 
//{
//    int index = 1;
//    for (auto it = m_TexMap.begin(); it != m_TexMap.end(); ++it, ++index)
//    {
//        if (it->first == texname)
//            return index;
//    }
//
//    //Safe_AddRef(texInstance);
//
//    m_TexMap[texname] = texInstance;
//
//    hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//    hDescriptor.Offset(index, CGameInstance::Get_Instance()->Get_CBVUAVSRVHeapSize());
//
//    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//    
//    
//
//    switch (TT)
//    {
//    case Engine::CTextureMgr::TT_TEXTURE2D:
//        
//        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//        srvDesc.Format = texInstance->Get_Texture()->GetDesc().Format;
//        srvDesc.Texture2D.MipLevels = texInstance->Get_Texture()->GetDesc().MipLevels;
//        srvDesc.Texture2D.MostDetailedMip = 0;
//        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
//        GETDEVICE->CreateShaderResourceView(texInstance->Get_Texture(), &srvDesc, hDescriptor);
//        break;
//
//    case Engine::CTextureMgr::TT_TEXTURECUBE:
//        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
//        srvDesc.TextureCube.MostDetailedMip = 0;
//        srvDesc.TextureCube.MipLevels = texInstance->Get_Texture()->GetDesc().MipLevels;
//        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
//        srvDesc.Format = texInstance->Get_Texture()->GetDesc().Format;
//        GETDEVICE->CreateShaderResourceView(texInstance->Get_Texture(), &srvDesc, hDescriptor);
//        break;
//
//    case Engine::CTextureMgr::TT_END:
//        break;
//    default:
//        break;
//    }
//
//    //hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//
//    return index;
//}
//
//void CTextureMgr::Add_ShadowMap(ID3D12Resource** Resource)
//{
//    hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//
//    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
//    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//    srvDesc.Texture2D.MostDetailedMip = 0;
//    srvDesc.Texture2D.MipLevels = 1;
//    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
//    srvDesc.Texture2D.PlaneSlice = 0;
//    GETDEVICE->CreateShaderResourceView(*Resource, &srvDesc, hDescriptor);
//}
//
//void CTextureMgr::Make_DescriptorHeap()
//{
//
//    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
//    srvHeapDesc.NumDescriptors = 500;
//    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//    ThrowIfFailed(GETDEVICE->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap)));
//
//    hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
//}
//
//void CTextureMgr::Set_DescriptorHeap()
//{
//    ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
//    GETCOMMANDLIST->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
//    //Todo : 나중에 커맨드리스트 가지고 있도록 만들기
//}
//
//void CTextureMgr::Free()
//{
//    for (auto& tex : m_TexMap)
//        Safe_Release(tex.second);
//}


#include "TextureMgr.h"
#include "GameInstance.h"

IMPLEMENT_SINGLETON(CTextureMgr);

// 텍스처 인덱스 = m_Textures 의 위치. 0번은 분홍 기본 텍스처라 실제 텍스처는 예전처럼 1번부터다.
// 힙 칸 = SRV_ENGINE_RESERVED + 텍스처 인덱스 (Engine_Config.h)

// 내부용. 등록 여부를 정확히 알아야 할 때 쓴다 (0번 분홍과 "없음"을 구분)
_int CTextureMgr::Find_Index(const string& texname) const
{
	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		if (m_Textures[i].first == texname)
			return static_cast<_int>(i);
	}
	return -1;
}

// 이름으로 등록된 텍스처를 찾는다. 없으면 0 (분홍 기본 텍스처) → 화면에서 바로 눈에 띈다
_uint CTextureMgr::Find_Texture(const string& texname) const
{
	_int index = Find_Index(texname);
	return index < 0 ? 0 : static_cast<_uint>(index);
}

// 분홍 기본 텍스처를 텍스처 인덱스 0번(힙 SRV_ENGINE_RESERVED 칸)에 등록한다.
// Make_DescriptorHeap 직후, 다른 텍스처보다 먼저 한 번만 부른다. 경로 기준은 작업 디렉터리 Client/MainApp/Default
void CTextureMgr::Add_DefaultTexture()
{
	assert(m_Textures.empty());
	Add_Texture("Engine_DefaultPink", L"../bin/Models/Default/DefaultPink.dds");
	// 분홍 로드에 실패하면 첫 실제 텍스처가 0번이 되어 인덱스가 전부 하나씩 밀린다
	assert(m_Textures.size() == 1 && "DefaultPink.dds load failed");
}

// 파일 경로를 받는 버전. 이미 등록된 이름이면 파일을 읽지 않고 인덱스만 돌려준다
_uint CTextureMgr::Add_Texture(const string& texname, const wchar_t* Filename, TEXTURETYPE TT)
{
	_int existing = Find_Index(texname);
	if (existing >= 0)
		return static_cast<_uint>(existing);

	return Add_Texture(texname, CTexture::Create(Filename), TT);
}

_uint CTextureMgr::Add_Texture(string texname, CTexture* texInstance, TEXTURETYPE TT)
{
	// [수정 전] 같은 이름이 있으면 인덱스만 반환하고 texInstance를 해제하지 않음 → CTexture와 GPU 텍스처 누수
	//// vector를 순회하여 이름이 같은 텍스처가 있는지 확인
	//for (size_t i = 0; i < m_Textures.size(); ++i)
	//{
	//	if (m_Textures[i].first == texname)
	//		return i + 1; // 기존처럼 1-based index 반환
	//}
	// Find_Texture 는 "없음"도 0(분홍)으로 돌려주므로 등록 여부는 Find_Index 로 본다
	_int existing = Find_Index(texname);
	if (existing >= 0)
	{
		Safe_Release(texInstance);
		return static_cast<_uint>(existing);
	}

	// 파일 로드 실패 (CTexture::Create 는 실패해도 객체를 돌려주고 리소스만 nullptr) → 분홍 기본 텍스처로 대신한다
	// 예전에는 여기서 Get_Texture()->GetDesc() 가 nullptr 접근으로 크래시했다
	if (nullptr == texInstance || nullptr == texInstance->Get_Texture())
	{
		Safe_Release(texInstance);
		return 0;
	}

	// 텍스처 구간이 꽉 찼으면 (셰이더 gTextureMaps 배열 크기 = MAX_TEXTURE_COUNT)
	if (m_Textures.size() >= MAX_TEXTURE_COUNT)
	{
		assert(!"TextureMgr: MAX_TEXTURE_COUNT exceeded");
		Safe_Release(texInstance);
		return 0;
	}

	// 존재하지 않으면 새로 추가
	m_Textures.push_back({ texname, texInstance });

	// 새 텍스처 인덱스 = vector 위치 (0번은 분홍이라 실제 텍스처는 1번부터)
	_uint newIndex = static_cast<_uint>(m_Textures.size() - 1);

	// 힙 칸 = 엔진 예약 구간 뒤 + 텍스처 인덱스
	hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	hDescriptor.Offset(SRV_ENGINE_RESERVED + newIndex, CGameInstance::Get_Instance()->Get_CBVUAVSRVHeapSize());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (TT)
	{
	case Engine::CTextureMgr::TT_TEXTURE2D:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = texInstance->Get_Texture()->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = texInstance->Get_Texture()->GetDesc().MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		GETDEVICE->CreateShaderResourceView(texInstance->Get_Texture(), &srvDesc, hDescriptor);
		break;

	case Engine::CTextureMgr::TT_TEXTURECUBE:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = texInstance->Get_Texture()->GetDesc().MipLevels;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		srvDesc.Format = texInstance->Get_Texture()->GetDesc().Format;
		GETDEVICE->CreateShaderResourceView(texInstance->Get_Texture(), &srvDesc, hDescriptor);
		break;

	case Engine::CTextureMgr::TT_END:
		break;
	default:
		break;
	}

	return newIndex;
}

void CTextureMgr::Add_ShadowMap(ID3D12Resource** Resource)
{
	// 엔진 예약 구간의 그림자맵 칸 (Engine_Config.h SRV_SLOT_SHADOWMAP)
	hDescriptor = Get_EngineSrvCpuHandle(SRV_SLOT_SHADOWMAP);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	GETDEVICE->CreateShaderResourceView(*Resource, &srvDesc, hDescriptor);
}

ID3D12Resource* CTextureMgr::Get_Texture(string texname)
{
	// vector를 순회하여 이름으로 텍스처 검색
	for (const auto& pair : m_Textures)
	{
		if (pair.first == texname)
			return pair.second->Get_Texture();
	}
	return nullptr; // 찾지 못하면 nullptr 반환
}

CD3DX12_CPU_DESCRIPTOR_HANDLE CTextureMgr::Get_EngineSrvCpuHandle(_uint slot) const
{
	assert(slot < SRV_ENGINE_RESERVED);
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		slot, CGameInstance::Get_Instance()->Get_CBVUAVSRVHeapSize());
}

CD3DX12_GPU_DESCRIPTOR_HANDLE CTextureMgr::Get_EngineSrvGpuHandle(_uint slot) const
{
	assert(slot < SRV_ENGINE_RESERVED);
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
		slot, CGameInstance::Get_Instance()->Get_CBVUAVSRVHeapSize());
}

CD3DX12_GPU_DESCRIPTOR_HANDLE CTextureMgr::Get_TextureTableGpuHandle() const
{
	return Get_TextureGpuHandle(0);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE CTextureMgr::Get_TextureGpuHandle(_uint texIndex) const
{
	assert(texIndex < MAX_TEXTURE_COUNT);
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
		SRV_ENGINE_RESERVED + texIndex, CGameInstance::Get_Instance()->Get_CBVUAVSRVHeapSize());
}


void CTextureMgr::Make_DescriptorHeap()
{
	// 엔진 예약 구간(16) + 텍스처 구간(500). slot 5 테이블이 16칸 뒤에서 500칸을 잡으므로 힙도 516칸이어야 한다
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = SRV_HEAP_SIZE;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(GETDEVICE->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap)));

	hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

void CTextureMgr::Set_DescriptorHeap()
{
	// (기존 코드와 동일)
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
	GETCOMMANDLIST->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
}

void CTextureMgr::Resize_TexMap()
{
	m_Textures.reserve(MAX_TEXTURE_COUNT);
}


void CTextureMgr::Free()
{
	for (auto& pair : m_Textures)
		Safe_Release(pair.second);

	m_Textures.clear();

	Safe_Release(srvDescriptorHeap);
}