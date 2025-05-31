#include "TextureMgr.h"
#include "GameInstance.h"

IMPLEMENT_SINGLETON(CTextureMgr);

int CTextureMgr::Add_Texture(string texname, CTexture* texInstance, TEXTURETYPE TT) 
{
    int index = 0;
    for (auto it = m_TexMap.begin(); it != m_TexMap.end(); ++it, ++index)
    {
        if (it->first == texname)
            return index;
    }

    Safe_AddRef(texInstance);

    m_TexMap[texname] = texInstance;

    hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    hDescriptor.Offset(index, CGameInstance::Get_Instance()->Get_CBVUAVSRVHeapSize());

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

    //hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    return index;
}

void CTextureMgr::Make_DescriptorHeap()
{

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 500;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(GETDEVICE->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap)));

    hDescriptor = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

void CTextureMgr::Set_DescriptorHeap()
{
    ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
    GETCOMMANDLIST->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    //Todo : 나중에 커맨드리스트 가지고 있도록 만들기
}

void CTextureMgr::Free()
{
    for (auto& tex : m_TexMap)
        Safe_Release(tex.second);
}
