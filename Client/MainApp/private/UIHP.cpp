#include "Client_pch.h"
#include "UIHP.h"
#include "GameInstance.h"

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

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UIHP", CTexture::Create(L"../bin/Models/UI/1.dds"));

    m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("UIHP", mat));

    m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

    m_TransformCom->Set_Scale(CTransform::STATE_UP, 1.77f);

    m_TransformCom->Set_Scale(2.f);

    m_TransformCom->Set_Scale(CTransform::STATE_RIGHT, HP/2.f);

    m_TransformCom->Set_Scale(CTransform::STATE_UP, 25);

    m_TransformCom->Set_Scale(0.01f);

    m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(-0.5f, -0.9f, 0.f, 1.f));

    //m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, -0.09f, 0.f, 1.f));

    //m_CBBinding->Set_Pad0(1);

   // m_isFPS = false;

    return S_OK;
}

void CUIHP::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);
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
