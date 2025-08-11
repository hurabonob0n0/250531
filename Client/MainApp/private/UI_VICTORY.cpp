#include "Client_pch.h"
#include "UI_VICTORY.h"
#include "GameInstance.h"

CUI_VICTORY::CUI_VICTORY() : CUIObject()
{
}

CUI_VICTORY::CUI_VICTORY(CUI_VICTORY& rhs) : CUIObject(rhs)
{

}

HRESULT CUI_VICTORY::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CUI_VICTORY::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_UI;

	__super::Initialize(pArg);

	MaterialData mat{};

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("VictoryUI", CTexture::Create(L"../bin/Models/FinalUI/VictoryUI.dds"));

	m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("VictoryUI", mat));

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_TransformCom->Set_Scale(CTransform::STATE_UP, 1.77f);

	m_TransformCom->Set_Scale(0.75f);

	//m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, -0.09f, 0.f, 1.f));

	//m_CBBinding->Set_Pad0(1);

   // m_isFPS = false;

	return S_OK;
}

void CUI_VICTORY::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CUI_VICTORY::LateTick(float fTimeDelta)
{
	__super::LateTick(fTimeDelta);
}

void CUI_VICTORY::Render()
{
	if (render)
	{
		__super::Render();
		m_VIBuffer->Render();
	}
}

void CUI_VICTORY::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CUI_VICTORY* CUI_VICTORY::Create()
{
	CUI_VICTORY* pInstance = new CUI_VICTORY;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CUI_VICTORY* CUI_VICTORY::Clone(void* pArg)
{
	CUI_VICTORY* pInstance = new CUI_VICTORY(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
