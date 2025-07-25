#include "FrameResource.h"
#include "GameInstance.h"

CFrameResource::CFrameResource()
{
}

void CFrameResource::Initialize(ID3D12Device* device)
{
    if (FAILED(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&m_CmdListAlloc))))
        MSG_BOX("Failed to Create : CommandListAllocator");

    m_PassCB = CUploadBuffer<PassConstants>::Create(device, 1, true);
    m_ObjectCB = CUploadBuffer<ObjectConstants>::Create(device, 1000, true);
    m_MaterialCB = CUploadBuffer<MaterialData>::Create(device, 1000, false);
}

void CFrameResource::Set_PassConstants()
{
    GETCOMMANDLIST->SetGraphicsRootConstantBufferView(1, m_PassCB->Resource()->GetGPUVirtualAddress());
}

void CFrameResource::Set_Materials()
{
    GETCOMMANDLIST->SetGraphicsRootShaderResourceView(2, m_MaterialCB->Resource()->GetGPUVirtualAddress());
}

void CFrameResource::Set_ObjectConstantBufferIndex(_uint& CBIndex)
{
    CBIndex = UsedObjectCBIndex;
    UsedObjectCBIndex += 1;
}

CFrameResource* CFrameResource::Create(ID3D12Device* device)
{
    CFrameResource* pInstance = new CFrameResource;
    pInstance->Initialize(device);
    return pInstance;
}

void CFrameResource::Free()
{
    Safe_Release(m_CmdListAlloc);
    Safe_Release(m_PassCB);
    Safe_Release(m_ObjectCB);
    Safe_Release(m_MaterialCB);
}

