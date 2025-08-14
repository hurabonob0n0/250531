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

_uint CTextureMgr::Add_Texture(string texname, CTexture* texInstance, TEXTURETYPE TT)
{
	// vector를 순회하여 이름이 같은 텍스처가 있는지 확인
	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		if (m_Textures[i].first == texname)
			return i + 1; // 기존처럼 1-based index 반환
	}

	// 존재하지 않으면 새로 추가
	m_Textures.push_back({ texname, texInstance });

	// 새 1-based index 계산
	_uint newIndex = m_Textures.size();

	hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	hDescriptor.Offset(newIndex, CGameInstance::Get_Instance()->Get_CBVUAVSRVHeapSize());

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
	// (기존 코드와 동일)
	hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

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


void CTextureMgr::Make_DescriptorHeap()
{
	// (기존 코드와 동일)
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 500;
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
	m_Textures.reserve(500);
}


void CTextureMgr::Free()
{
	for (auto& pair : m_Textures)
		Safe_Release(pair.second);

	m_Textures.clear();
}