#include "Client_pch.h"
#include "UITeamPercent.h"
#include "GameInstance.h"

CUITeamPercent::CUITeamPercent() : CUIObject()
{
}

CUITeamPercent::CUITeamPercent(CUITeamPercent& rhs) : CUIObject(rhs)
{

}

HRESULT CUITeamPercent::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CUITeamPercent::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_UI;

	__super::Initialize(pArg);

	MaterialData mat{};

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UITeamPercent", CTexture::Create(L"../bin/Models/UI/5-3.dds"));

	BlankMatIndex = m_GameInstance->Add_Material("UITeamPercentBlank", mat);

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UIRedTeamPercent", CTexture::Create(L"../bin/Models/UI/5-1.dds"));

	RedTeamIndex = m_GameInstance->Add_Material("UIRedTeamPercent", mat);

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("UIBlueTeamPercent", CTexture::Create(L"../bin/Models/UI/5-2.dds"));

	BlueTeamIndex = m_GameInstance->Add_Material("UIBlueTeamPercent", mat);

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_BlueTeamBlank = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(-0.5f, 0.5f, 0.f);

	m_RedTeamBlank = XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(+0.5f, 0.5f, 0.f);

	//m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, -0.09f, 0.f, 1.f));

	m_CBBinding->Set_Pad0(2);

   // m_isFPS = false;

	return S_OK;
}

void CUITeamPercent::Tick(float fTimeDelta)
{
	//__super::Tick(fTimeDelta);
}

void CUITeamPercent::LateTick(float fTimeDelta)
{
	//__super::LateTick(fTimeDelta);
}

void CUITeamPercent::Render()
{
	m_CBBinding->Set_CBIndex();
	m_CBBinding->Set_MaterialIndex(BlankMatIndex);
	m_CBBinding->Set_WorldMatrix(m_BlueTeamBlank);
	m_CBBinding->Set_TexCoordMatrix(m_TransformCom->Get_WorldMatrix());
	m_CBBinding->Set_Pad1(100);
	m_CBBinding->Update_CBView();
	m_CBBinding->Set_On_Shader();
	m_VIBuffer->Render();

	m_CBBinding->Set_CBIndex();
	m_CBBinding->Set_MaterialIndex(BlankMatIndex);
	m_CBBinding->Set_WorldMatrix(m_RedTeamBlank);
	m_CBBinding->Set_TexCoordMatrix(m_TransformCom->Get_WorldMatrix());
	m_CBBinding->Set_Pad1(100);
	m_CBBinding->Update_CBView();
	m_CBBinding->Set_On_Shader();
	m_VIBuffer->Render();

}

void CUITeamPercent::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CUITeamPercent* CUITeamPercent::Create()
{
	CUITeamPercent* pInstance = new CUITeamPercent;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CUITeamPercent* CUITeamPercent::Clone(void* pArg)
{
	CUITeamPercent* pInstance = new CUITeamPercent(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
