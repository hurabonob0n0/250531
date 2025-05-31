#include "CBBinding.h"
#include "GameInstance.h"

CBBinding::CBBinding(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, CFrameResourceMgr* pFrameResource) : CComponent(pDevice,pCommandList)
{
    m_FrameResourceMgr = pFrameResource;
    Safe_AddRef(m_FrameResourceMgr);
}

CBBinding::CBBinding(CBBinding& rhs) : CComponent(rhs)
{
    m_FrameResourceMgr = rhs.m_FrameResourceMgr;
    Safe_AddRef(m_FrameResourceMgr);
}

void CBBinding::Set_CBIndex()
{
    m_FrameResourceMgr->Set_ObjectConstantBufferIndex(m_ObjCBIndex);
}

void CBBinding::Set_WorldMatrix(CTransform* pTransformCom)
{
    XMStoreFloat4x4(&m_ObjConstants.World, XMMatrixTranspose(pTransformCom->Get_WorldMatrix()));
}

void CBBinding::Set_WorldMatrix(_matrix worldmat)
{
    XMStoreFloat4x4(&m_ObjConstants.World, XMMatrixTranspose(worldmat));
}

void CBBinding::Set_WorldMatrix(_float4x4 worldMat)
{
    XMStoreFloat4x4(&m_ObjConstants.World, XMMatrixTranspose(XMLoadFloat4x4(&worldMat)));
}

void CBBinding::Set_TexCoordMatrix(CTransform* pTexCoordTransformCom)
{
    XMStoreFloat4x4(&m_ObjConstants.TexTransform, XMMatrixTranspose(pTexCoordTransformCom->Get_WorldMatrix()));
}

void CBBinding::Set_TexCoordMatrix(_matrix TexCoordmat)
{
    XMStoreFloat4x4(&m_ObjConstants.TexTransform, XMMatrixTranspose(TexCoordmat));
}

void CBBinding::Set_MaterialIndex(_uint MatIndex)
{
    m_ObjConstants.MaterialIndex = MatIndex;
}

void CBBinding::Set_World_TexCoord_And_Update(CTransform* pTransformCom, CTransform* pTexCoordTransformCom)
{
    Set_CBIndex();
    Set_WorldMatrix(pTransformCom);
    Set_TexCoordMatrix(pTexCoordTransformCom);

    m_FrameResourceMgr->Get_Current_FrameResource()->m_ObjectCB->CopyData(m_ObjCBIndex, m_ObjConstants);
}

void CBBinding::Update_CBView()
{
    m_ObjConstants;

    m_FrameResourceMgr->Get_Current_FrameResource()->m_ObjectCB->CopyData(m_ObjCBIndex, m_ObjConstants);
}

void CBBinding::Set_On_Shader()
{
    UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));

    D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = m_FrameResourceMgr->Get_Current_FrameResource()->m_ObjectCB->Resource()->GetGPUVirtualAddress();

    objCBAddress += m_ObjCBIndex * objCBByteSize;

    m_ObjConstants;

    m_CommandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
}

CBBinding* CBBinding::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, CFrameResourceMgr* pFrameResource)
{
    CBBinding* pInstance = new CBBinding(pDevice,pCommandList,pFrameResource);
    return pInstance;
}

CComponent* CBBinding::Clone(void* pArg)
{
    CBBinding* pInstance = new CBBinding(*this);
    return pInstance;
}

void CBBinding::Free()
{
    __super::Free();
    Safe_Release(m_FrameResourceMgr);
}