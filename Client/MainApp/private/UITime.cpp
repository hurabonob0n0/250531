#include "Client_pch.h"
#include "UITime.h"
#include "UINumber.h"
#include "GameInstance.h"
#include "Network_Manager.h"

CUITime::CUITime() : CRenderObject()
{
}

CUITime::CUITime(CUITime& rhs) : CRenderObject(rhs)
{

}

HRESULT CUITime::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CUITime::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_UI;

    __super::Initialize(pArg);

    TenMinutes = (CUINumber*)m_GameInstance->GetGameObject("UINumber", 0);
    OneMinutes = (CUINumber*)m_GameInstance->GetGameObject("UINumber", 1);
    TenSeconds = (CUINumber*)m_GameInstance->GetGameObject("UINumber", 2);
    OneSeconds = (CUINumber*)m_GameInstance->GetGameObject("UINumber", 3);

    Safe_AddRef(TenMinutes);
    Safe_AddRef(OneMinutes);
    Safe_AddRef(TenSeconds);
    Safe_AddRef(OneSeconds);

    return S_OK;
}

void CUITime::Tick(float fTimeDelta)
{

    if(Active == true)
        Update_Time(fTimeDelta);
    else {
        if (Network_Manager::GetInstance()->GetGameStart())
            Active = true;
    }
}

void CUITime::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);
}

void CUITime::Render()
{
    _uint iGameTime = (int)fGameTime;
    _uint Tenmins = iGameTime / 600;
    iGameTime -= 600 * Tenmins;
    _uint Onemins = iGameTime / 60;
    iGameTime -= 60 * Onemins;
    _uint Tensecs = iGameTime / 10;
    iGameTime -= 10 * Tensecs;
    _uint Onesecs = iGameTime;

    TenMinutes->Set_World_Identity();
    TenMinutes->Set_Pos(1800.f - 50.f - 20.f, 1040.f);
    TenMinutes->Set_Scale(0.05f, 0.05f * 2.f);
    TenMinutes->Set_Number(Tenmins);
    TenMinutes->Set_On_CBBinding();
    TenMinutes->Render();

    OneMinutes->Set_World_Identity();
    OneMinutes->Set_Pos(1800.f - 50.f + 20.f, 1040.f);
    OneMinutes->Set_Scale(0.05f, 0.05f * 2.f);
    OneMinutes->Set_Number(Onemins);
    OneMinutes->Set_On_CBBinding();
    OneMinutes->Render();

    TenSeconds->Set_World_Identity();
    TenSeconds->Set_Pos(1800.f + 50.f - 20.f, 1040.f);
    TenSeconds->Set_Scale(0.05f, 0.05f * 2.f);
    TenSeconds->Set_Number(Tensecs);
    TenSeconds->Set_On_CBBinding();
    TenSeconds->Render();                                                                              

    OneSeconds->Set_World_Identity();            
    OneSeconds->Set_Pos(1800.f + 50.f + 20.f, 1040.f);
    OneSeconds->Set_Scale(0.05f, 0.05f * 2.f);
    OneSeconds->Set_Number(Onesecs);
    OneSeconds->Set_On_CBBinding();
    OneSeconds->Render();
}

void CUITime::Update_Time(float fTimeDelta)
{
    fGameTime += fTimeDelta;
}

void CUITime::Free()
{
    Safe_Release(TenMinutes);
    Safe_Release(OneMinutes);
    Safe_Release(TenSeconds);
    Safe_Release(OneSeconds);

    __super::Free();
}

CUITime* CUITime::Create()
{
    CUITime* pInstance = new CUITime;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CUITime* CUITime::Clone(void* pArg)
{
    CUITime* pInstance = new CUITime(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}