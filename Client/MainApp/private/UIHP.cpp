#include "Client_pch.h"
#include "UIHP.h"
#include "GameInstance.h"
#include "Network_Manager.h"
#include "UINumber.h"

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

    _100Hp =    (CUINumber*)m_GameInstance->GetGameObject("UINumber", 4);
    _10Hp =     (CUINumber*)m_GameInstance->GetGameObject("UINumber", 5);
    _1Hp =      (CUINumber*)m_GameInstance->GetGameObject("UINumber", 6);

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

    int Hp = Network_Manager::GetInstance()->MyHp;
    int hundred = Hp / 100;
    Hp -= 100 * hundred;
    int ten = Hp / 10;
    Hp -= 10 * ten;
    int one = Hp;

    _100Hp->Set_World_Identity();
    _100Hp->Set_Pos(180, 100.f);
    _100Hp->Set_Scale(0.035f, 0.035f * 2.2f);
    _100Hp->Set_Number(hundred);
    _100Hp->Set_On_CBBinding();
    _100Hp->Render();

    _10Hp->Set_World_Identity();
    _10Hp->Set_Pos(205, 100.f);
    _10Hp->Set_Scale(0.035f, 0.035f * 2.2f);
    _10Hp->Set_Number(ten);
    _10Hp->Set_On_CBBinding();
    _10Hp->Render();

    _1Hp->Set_World_Identity();
    _1Hp->Set_Pos(230, 100.f);
    _1Hp->Set_Scale(0.035f, 0.035f * 2.2f);
    _1Hp->Set_Number(one);
    _1Hp->Set_On_CBBinding();
    _1Hp->Render();
}

void CUIHP::Free()
{
    Safe_Release(m_VIBuffer);
    Safe_Release(_100Hp);
    Safe_Release(_10Hp);
    Safe_Release(_1Hp);

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
