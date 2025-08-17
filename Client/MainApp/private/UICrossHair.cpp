#include "Client_pch.h"
#include "UICrossHair.h"
#include "GameInstance.h"
#include "StateMgr.h"

CUICrossHair::CUICrossHair() : CUIObject()
{
}

CUICrossHair::CUICrossHair(CUICrossHair& rhs) : CUIObject(rhs)
{
    
}

HRESULT CUICrossHair::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CUICrossHair::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_UI;

    __super::Initialize(pArg);

    MaterialData mat{};

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("TPSAIM", CTexture::Create(L"../bin/Models/FinalUI/TPSAIM.dds"));

    TPSMatIndex = m_GameInstance->Add_Material("TPSAIM", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("FPSCrossHair", CTexture::Create(L"../bin/Models/FinalUI/FPSCrossHair.dds"));

    FPSMatIndex = m_GameInstance->Add_Material("FPSCrossHair", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("DroneCrossHair", CTexture::Create(L"../bin/Models/FinalUI/ForDroneUI.dds"));

    DroneMatIndex = m_GameInstance->Add_Material("DroneCrossHair", mat);

    m_CBBinding->Set_MaterialIndex(FPSMatIndex);
    m_TransformCom->Identity();
    __super::Set_Scale(2.f, 2.f * 1.7777f);

    m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

    return S_OK;
}

void CUICrossHair::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);
    Update_State();
}

void CUICrossHair::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);
}

void CUICrossHair::Render()
{
    __super::Render();
    m_VIBuffer->Render();
}

void CUICrossHair::Update_State()
{
    GameMode GM = CStateMgr::Get_GameMode();

    switch (GM)
    {
    case GM_TPS:
        m_CBBinding->Set_MaterialIndex(TPSMatIndex);
        m_TransformCom->Identity();
        __super::Set_Scale(1.f, 1.f);
        break;
    case GM_FPS:
        m_CBBinding->Set_MaterialIndex(FPSMatIndex);
        m_TransformCom->Identity();
        __super::Set_Scale(2.f, 2.f * 1.7777f);
        break;
    case GM_Drone:
        m_CBBinding->Set_MaterialIndex(DroneMatIndex);
        m_TransformCom->Identity();
        __super::Set_Scale(1.f,  1.7777f);
        break;
    }
}

void CUICrossHair::Free()
{
    Safe_Release(m_VIBuffer);

    __super::Free();
}

CUICrossHair* CUICrossHair::Create()
{
    CUICrossHair* pInstance = new CUICrossHair;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CUICrossHair* CUICrossHair::Clone(void* pArg)
{
    CUICrossHair* pInstance = new CUICrossHair(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}
