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

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Crosshair", CTexture::Create(L"../bin/Models/Crosshair/Crosshair1.dds"));

    m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("UIMat", mat));

    m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

    m_TransformCom->Set_Scale(2.f);

    m_isFPS = false;

    return S_OK;
}

void CUI::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);
    if (m_GameInstance->Key_Down('M'))
        m_isFPS = !m_isFPS;
}

void CUI::LateTick(float fTimeDelta)
{
    if(m_isFPS)
        __super::LateTick(fTimeDelta);
}

void CUI::Render()
{
    if (m_isFPS)
    __super::Render();
    if (m_isFPS)
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
