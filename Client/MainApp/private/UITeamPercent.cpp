#include "Client_pch.h"
#include "UITeamPercent.h"
#include "GameInstance.h"
#include "Network_Manager.h"

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

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("CaptureUI", CTexture::Create(L"../bin/Models/FinalUI/CaptureUI.dds"));

	BlankMatIndex = m_GameInstance->Add_Material("CaptureUI", mat);

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("BlueCaptureUI", CTexture::Create(L"../bin/Models/FinalUI/BlueCaptureUI.dds"));

	BlueMatIndex = m_GameInstance->Add_Material("BlueCaptureUI", mat);

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("RedCaptureUI", CTexture::Create(L"../bin/Models/FinalUI/RedCaptureUI.dds"));

	RedMatIndex = m_GameInstance->Add_Material("RedCaptureUI", mat);

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_CBBinding->Set_TexCoordMatrix(XMMatrixIdentity());

	m_CBBinding->Set_Pad0(2);

	return S_OK;
}

void CUITeamPercent::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

void CUITeamPercent::LateTick(float fTimeDelta)
{
	__super::LateTick(fTimeDelta);
}

void CUITeamPercent::Render()
{
	m_TransformCom->Identity();
	__super::Set_Scale(BlankScale.x, BlankScale.y);
	__super::Set_Pos(BlankPos.x, BlankPos.y);

	m_CBBinding->Set_CBIndex();
	m_CBBinding->Set_MaterialIndex(BlankMatIndex);
	m_CBBinding->Set_WorldMatrix(m_TransformCom);
	m_CBBinding->Set_Pad1(0);
	m_CBBinding->Update_CBView();
	m_CBBinding->Set_On_Shader();
	m_VIBuffer->Render();

	m_TransformCom->Identity();
	__super::Set_Scale(BlueTeamScale.x, BlueTeamScale.y);
	__super::Set_Pos(BlueTeamPos.x, BlueTeamPos.y);

	m_CBBinding->Set_CBIndex();
	m_CBBinding->Set_MaterialIndex(BlueMatIndex);
	m_CBBinding->Set_WorldMatrix(m_TransformCom);
	m_CBBinding->Set_Pad1(1);
	m_CBBinding->Set_Pad2((float)Network_Manager::GetInstance()->BLUEBAR); 
	m_CBBinding->Update_CBView();
	m_CBBinding->Set_On_Shader();
	m_VIBuffer->Render();

	m_TransformCom->Identity();
	__super::Set_Scale(RedTeamScale.x, RedTeamScale.y);
	__super::Set_Pos(RedTeamPos.x, RedTeamPos.y);

	m_CBBinding->Set_CBIndex();
	m_CBBinding->Set_MaterialIndex(RedMatIndex);
	m_CBBinding->Set_WorldMatrix(m_TransformCom);
	m_CBBinding->Set_Pad1(2);
	m_CBBinding->Set_Pad2((float)Network_Manager::GetInstance()->REDBAR);
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
