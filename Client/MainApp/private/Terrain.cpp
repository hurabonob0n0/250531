#include "Client_pch.h"
#include "Terrain.h"
#include "Client_Defines.h"
#include "GameInstance.h"

CTerrain::CTerrain() : CRenderObject()
{
}

CTerrain::CTerrain(CTerrain& rhs) : CRenderObject(rhs)
{
}

HRESULT CTerrain::Initialize_Prototype()
{
    __super::Initialize_Prototype();
    return S_OK;
}

HRESULT CTerrain::Initialize(void* pArg)
{
    __super::Initialize(pArg);

    m_RG = CRenderer::RG_BLEND;

    m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);
    
    m_VIBufferCom = (CVIBuffer_Terrain*)m_GameInstance->Get_Component("TerrainCom");

    return S_OK;
}

void CTerrain::Tick(float fTimeDelta)
{
    

    __super::Tick(fTimeDelta);
}

void CTerrain::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);

    m_CBBindingCom->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);
}

void CTerrain::Render()
{
    m_CBBindingCom->Set_On_Shader();

    m_VIBufferCom->Render();
}

CTerrain* CTerrain::Create()
{
    CTerrain* pInstance = new CTerrain;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CRenderObject* CTerrain::Clone(void* pArg)
{
    CTerrain* pInstance = new CTerrain(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}

void CTerrain::Free()
{
    Safe_Release(m_VIBufferCom);
    Safe_Release(m_CBBindingCom);
    __super::Free();
}
