#include "Client_pch.h"
#include "UISkillBox.h"
#include "GameInstance.h"
#include "StateMgr.h"
#include "Tank.h"
#include "Drone.h"
#include "Network_Manager.h"
CUISkillBox::CUISkillBox()
{
}

CUISkillBox::CUISkillBox(CUISkillBox& rhs)
{
}

HRESULT CUISkillBox::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CUISkillBox::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_UI;

	__super::Initialize(pArg);

	MaterialData mat{};
	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("AIRSTRIKESKILL", CTexture::Create(L"../bin/Models/FinalUI/AIRSTRIKESKILL.dds"));
	AIRSTRIKESKILL_BoxMat = m_GameInstance->Add_Material("AIRSTRIKESKILL", mat);	

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("DRONESKILL", CTexture::Create(L"../bin/Models/FinalUI/DRONESKILL.dds"));
	DRONESKILL_BoxMat = m_GameInstance->Add_Material("DRONESKILL", mat);	

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("FALLOWSKILL", CTexture::Create(L"../bin/Models/FinalUI/FALLOWSKILL.dds"));
	FALLOWSKILL_BoxMat = m_GameInstance->Add_Material("FALLOWSKILL", mat);	

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("CoolTimeBoxUI", CTexture::Create(L"../bin/Models/FinalUI/CoolTimeBoxUI.dds"));
	COLLTIME_BoxMat = m_GameInstance->Add_Material("CoolTimeBoxUI", mat);

	m_CBBinding->Set_MaterialIndex(COLLTIME_BoxMat);
	m_TransformCom->Identity();
	__super::Set_Scale(2.f, 2.f * 1.7777f);

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	return S_OK;
}

void CUISkillBox::Tick(float fTimeDelta)
{
	__super::Tick(fTimeDelta);

}

void CUISkillBox::LateTick(float fTimeDelta)
{

	__super::LateTick(fTimeDelta);
}

void CUISkillBox::Render()
{
	GameMode GM = CStateMgr::Get_GameMode();
	switch (GM)
	{
	case GM_TPS:

		break;
	case GM_FPS:
	{
		m_TransformCom->Identity();
		__super::Set_Scale(SkillboxScaleX, defaultScaleY * SkillboxScaleX);
		__super::Set_Pos(150.f, 600.f);
		m_CBBinding->Set_CBIndex();
		m_CBBinding->Set_MaterialIndex(AIRSTRIKESKILL_BoxMat);
		m_CBBinding->Set_WorldMatrix(m_TransformCom);
		m_CBBinding->Update_CBView();
		m_CBBinding->Set_On_Shader();
		m_VIBuffer->Render();

		float scale = dynamic_cast<CTank*>((m_GameInstance->Get_Instance()->GetGameObject("Tank", Network_Manager::GetInstance()->GetMyTankIndex())))->GetCoolScale();
		m_TransformCom->Identity();
		__super::Set_Scale(SkillboxScaleX, defaultScaleY * SkillboxScaleX * scale);
		__super::Set_Pos(150.f, 600.f);
		m_CBBinding->Set_CBIndex();
		m_CBBinding->Set_MaterialIndex(COLLTIME_BoxMat);
		m_CBBinding->Set_WorldMatrix(m_TransformCom);
		m_CBBinding->Update_CBView();
		m_CBBinding->Set_On_Shader();
		m_VIBuffer->Render();

		m_TransformCom->Identity();
		__super::Set_Scale(SkillboxScaleX, defaultScaleY * SkillboxScaleX);
		__super::Set_Pos(150.f, 400.f);
		m_CBBinding->Set_CBIndex();
		m_CBBinding->Set_MaterialIndex(DRONESKILL_BoxMat);
		m_CBBinding->Set_WorldMatrix(m_TransformCom);
		m_CBBinding->Update_CBView();
		m_CBBinding->Set_On_Shader();
		m_VIBuffer->Render();
	}


		break;
	case GM_Drone:
		m_TransformCom->Identity();
		__super::Set_Scale(SkillboxScaleX, defaultScaleY* SkillboxScaleX);
		__super::Set_Pos(150.f, 600.f);
		m_CBBinding->Set_CBIndex();
		m_CBBinding->Set_MaterialIndex(FALLOWSKILL_BoxMat);
		m_CBBinding->Set_WorldMatrix(m_TransformCom);
		m_CBBinding->Update_CBView();
		m_CBBinding->Set_On_Shader();
		m_VIBuffer->Render();
		
		
		
		if (dynamic_cast<CDrone*>((m_GameInstance->Get_Instance()->GetGameObject("Drone", Network_Manager::GetInstance()->GetMyTankIndex())))->m_followTank) {

			m_TransformCom->Identity();
			__super::Set_Scale(SkillboxScaleX, defaultScaleY * SkillboxScaleX);
			__super::Set_Pos(150.f, 600.f);
			m_CBBinding->Set_CBIndex();
			m_CBBinding->Set_MaterialIndex(COLLTIME_BoxMat);
			m_CBBinding->Set_WorldMatrix(m_TransformCom);
			m_CBBinding->Update_CBView();
			m_CBBinding->Set_On_Shader();
			m_VIBuffer->Render();
		}
	

		break;
	}


}


void CUISkillBox::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CUISkillBox* CUISkillBox::Create()
{
	CUISkillBox* pInstance = new CUISkillBox;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CUISkillBox* CUISkillBox::Clone(void* pArg)
{
	CUISkillBox* pInstance = new CUISkillBox(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}

