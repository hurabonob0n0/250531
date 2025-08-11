#include "Client_pch.h"
#include "AirDrop.h"
#include "GameInstance.h"

CUI_AirDrop::CUI_AirDrop() : CUIObject()
{
}

CUI_AirDrop::CUI_AirDrop(CUI_AirDrop& rhs) : CUIObject(rhs)
{

}

HRESULT CUI_AirDrop::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CUI_AirDrop::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_UI;

	__super::Initialize(pArg);

	MaterialData mat{};

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UIAirDrop", CTexture::Create(L"../bin/Models/FinalUI/AIRSTRIKEUI.dds"));

	m_CBBinding->Set_MaterialIndex(m_GameInstance->Add_Material("UIAirDrop", mat));

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_TransformCom->Set_Scale(CTransform::STATE_UP, 1.77f);

	m_TransformCom->Set_Scale(0.75f);

	//m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, -0.09f, 0.f, 1.f));

	//m_CBBinding->Set_Pad0(1);

   // m_isFPS = false;

	return S_OK;
}

void CUI_AirDrop::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CUI_AirDrop::LateTick(float fTimeDelta)
{
	__super::LateTick(fTimeDelta);
}

void CUI_AirDrop::Render()
{
	if (render)
	{
		__super::Render();
		m_VIBuffer->Render();
	}
}

void CUI_AirDrop::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CUI_AirDrop* CUI_AirDrop::Create()
{
	CUI_AirDrop* pInstance = new CUI_AirDrop;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CUI_AirDrop* CUI_AirDrop::Clone(void* pArg)
{
	CUI_AirDrop* pInstance = new CUI_AirDrop(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
