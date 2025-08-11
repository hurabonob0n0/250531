#include "Client_pch.h"
#include "UIHP.h"
#include "GameInstance.h"
#include "Network_Manager.h"

CUIHP::CUIHP() : CUIObject()
{
}

CUIHP::CUIHP(CUIHP& rhs) : CUIObject(rhs)
{
    
}

HRESULT CUIHP::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CUIHP::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_UI;

    __super::Initialize(pArg);

    MaterialData mat{};

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("HPBar", CTexture::Create(L"../bin/Models/FinalUI/HPBar.dds"));

    m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("HPBar", mat));

    m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

    m_CBBinding->Set_Pad0(1);

    __super::Set_Scale(0.75f, 0.1f);
    __super::Set_Pos(480, 100.f);



    return S_OK;
}

void CUIHP::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);

    m_CBBinding->Set_Pad2((float)Network_Manager::GetInstance()->MyHp);
}

void CUIHP::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);
}

void CUIHP::Render()
{
    __super::Render();
    m_VIBuffer->Render();
}

void CUIHP::Free()
{
    Safe_Release(m_VIBuffer);

    __super::Free();
}

CUIHP* CUIHP::Create()
{
    CUIHP* pInstance = new CUIHP;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CUIHP* CUIHP::Clone(void* pArg)
{
    CUIHP* pInstance = new CUIHP(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}
