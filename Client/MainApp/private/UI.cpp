#include "Client_pch.h"
#include "UI.h"
#include "GameInstance.h"

CUI::CUI() : CUIObject()
{
}

CUI::CUI(CUI& rhs) : CUIObject(rhs)
{
    
}

HRESULT CUI::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CUI::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_UI;

    __super::Initialize(pArg);

    MaterialData mat{};

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UITPS", CTexture::Create(L"../bin/Models/UI/3.dds"));

    TPSMatIndex = m_GameInstance->Add_Material("UITPS", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UIFPS", CTexture::Create(L"../bin/Models/UI/FPSUI.dds"));

    FPSMatIndex = m_GameInstance->Add_Material("UIFPS", mat);

    m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

    m_TransformCom->Set_Scale(CTransform::STATE_UP, 1.77f);

    m_TransformCom->Set_Scale(2.f);

    //m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, -0.09f, 0.f, 1.f));

    m_CBBinding->Set_Pad0(1);

    m_isFPS = false;

    return S_OK;
}

void CUI::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);
    if (m_GameInstance->Mouse_Down(1))
        m_isFPS = !m_isFPS;
}

void CUI::LateTick(float fTimeDelta)
{
    if (m_isFPS)
        m_CBBinding->Set_MaterialIndex(FPSMatIndex);
    else
        m_CBBinding->Set_MaterialIndex(TPSMatIndex);

    __super::LateTick(fTimeDelta);
}

void CUI::Render()
{
    __super::Render();
    m_VIBuffer->Render();
}

void CUI::Free()
{
    Safe_Release(m_VIBuffer);

    __super::Free();
}

CUI* CUI::Create()
{
    CUI* pInstance = new CUI;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CUI* CUI::Clone(void* pArg)
{
    CUI* pInstance = new CUI(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}
