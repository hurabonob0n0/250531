#include "Client_pch.h"
#include "UIKill.h"
#include "GameInstance.h"
#include "Network_Manager.h"

CUIKill::CUIKill() : CUIObject()
{
}

CUIKill::CUIKill(CUIKill& rhs) : CUIObject(rhs)
{

}

HRESULT CUIKill::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CUIKill::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_UI;

    __super::Initialize(pArg);

    MaterialData mat{};

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UIKill", CTexture::Create(L"../bin/Models/UI/9.dds"));

    m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("UIKill", mat));

    m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

    m_TransformCom->Set_Scale(0.1f);
    m_TransformCom->Set_Scale(CTransform::STATE_UP, 1.777f);

    return S_OK;
}

void CUIKill::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);

   /* if (m_GameInstance->Key_Down(VK_SPACE))
        isHit = true;*/

    if (isHit)
    {
        deltatime += fTimeDelta;
        if (deltatime >= cooltime)
        {
            isHit = false;
            deltatime = 0.f;
        }
    }
}

void CUIKill::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);
}

void CUIKill::Render()
{
    if (isHit)
    {
        __super::Render();
        m_VIBuffer->Render();
    }
}

void CUIKill::reset_deltatime()
{
    deltatime = 0.f;
}

void CUIKill::Free()
{
    Safe_Release(m_VIBuffer);

    __super::Free();
}

CUIKill* CUIKill::Create()
{
    CUIKill* pInstance = new CUIKill;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CUIKill* CUIKill::Clone(void* pArg)
{
    CUIKill* pInstance = new CUIKill(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}
