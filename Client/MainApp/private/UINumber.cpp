#include "Client_pch.h"
#include "UINumber.h"
#include "GameInstance.h"
#include "Network_Manager.h"

CUINumber::CUINumber() : CUIObject()
{
}

CUINumber::CUINumber(CUINumber& rhs) : CUIObject(rhs)
{
    
}

HRESULT CUINumber::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CUINumber::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_UI;

    __super::Initialize(pArg);

    MaterialData mat{};

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("NumberUI", CTexture::Create(L"../bin/Models/FinalUI/NumUI60X90.dds"));

    m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("NumberUI", mat));

    m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");
    
   

    return S_OK;
}

void CUINumber::Tick(float fTimeDelta)
{
    //__super::Tick(fTimeDelta);
}

void CUINumber::LateTick(float fTimeDelta)
{
    //__super::LateTick(fTimeDelta);
}

void CUINumber::Render()
{
    m_VIBuffer->Render();
}

void CUINumber::Set_World_Identity()
{
    m_TransformCom->Identity();
}

void CUINumber::Set_On_CBBinding()
{
    m_CBBinding->Set_World_TexCoord_And_Update(m_TransformCom,m_TexCoordTransformCom);
    m_CBBinding->Set_On_Shader();
}

void CUINumber::Free()
{
    Safe_Release(m_VIBuffer);

    __super::Free();
}

CUINumber* CUINumber::Create()
{
    CUINumber* pInstance = new CUINumber;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CUINumber* CUINumber::Clone(void* pArg)
{
    CUINumber* pInstance = new CUINumber(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}
