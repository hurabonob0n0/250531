#include "Client_pch.h"
#include "UI_DEFEAT.h"
#include "GameInstance.h"

CUI_DEFEAT::CUI_DEFEAT() : CUIObject()
{
}

CUI_DEFEAT::CUI_DEFEAT(CUI_DEFEAT& rhs) : CUIObject(rhs)
{

}

HRESULT CUI_DEFEAT::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CUI_DEFEAT::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_UI;

	__super::Initialize(pArg);

	MaterialData mat{};

	
	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("DefeatUI", CTexture::Create(L"../bin/Models/FinalUI/DefeatUI.dds"));

	m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("DefeatUI", mat));

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_TransformCom->Set_Scale(CTransform::STATE_UP, 1.77f);

	m_TransformCom->Set_Scale(0.75f);

	//m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, -0.09f, 0.f, 1.f));

	//m_CBBinding->Set_Pad0(1);

   // m_isFPS = false;

	return S_OK;
}

void CUI_DEFEAT::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CUI_DEFEAT::LateTick(float fTimeDelta)
{
	__super::LateTick(fTimeDelta);
}

void CUI_DEFEAT::Render()
{
	if (render)
	{
		__super::Render();
		m_VIBuffer->Render();
	}
}

void CUI_DEFEAT::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CUI_DEFEAT* CUI_DEFEAT::Create()
{
	CUI_DEFEAT* pInstance = new CUI_DEFEAT;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CUI_DEFEAT* CUI_DEFEAT::Clone(void* pArg)
{
	CUI_DEFEAT* pInstance = new CUI_DEFEAT(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
