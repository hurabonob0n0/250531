#include "Client_pch.h"
#include "WinningTeam.h"
#include "GameInstance.h"

CWinningTeam::CWinningTeam() : CRenderObject()
{
}

CWinningTeam::CWinningTeam(CWinningTeam& rhs) : CRenderObject(rhs)
{
}

HRESULT CWinningTeam::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CWinningTeam::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_NONBLEND;

    __super::Initialize(pArg);

    m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);

    CVIBuffer_Geos::BASIC_SUBMESHES mesh = CVIBuffer_Geos::BS_SPHERE;

    m_VIBuffer = (CVIBuffer_Geos*)m_GameInstance->Get_Component("VIBuffer_GeosCom",&mesh);

    return S_OK;
}

void CWinningTeam::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);

    //m_TransformCom->Go_Straight(fTimeDelta * 10.f);
}

void CWinningTeam::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);

    m_CBBindingCom->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);
}

void CWinningTeam::Render()
{
    m_CBBindingCom->Set_On_Shader();

    m_VIBuffer->Render();
}

void CWinningTeam::Free()
{
    Safe_Release(m_VIBuffer);
    Safe_Release(m_CBBindingCom);

    __super::Free();
}

CWinningTeam* CWinningTeam::Create()
{
    CWinningTeam* pInstance = new CWinningTeam;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CRenderObject* CWinningTeam::Clone(void* pArg)
{
    CWinningTeam* pInstance = new CWinningTeam(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}
