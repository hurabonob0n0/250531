#include "Client_pch.h"
#include "Zet.h"
#include "GameInstance.h"

CZet::CZet() : CRenderObject()
{
}

CZet::CZet(CZet& rhs) : CRenderObject(rhs)
{
}

HRESULT CZet::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CZet::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_NONLIGHT;

	__super::Initialize(pArg);

	m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);

	m_VIBuffer = (CMeshModel*)m_GameInstance->Get_Component("ZetModel");

	MaterialData MD{};

	XMStoreFloat4x4(&MD.MatTransform, XMMatrixIdentity());
	MD.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("ZetD", CTexture::Create(L"../bin/Models/Zet/Zet_D.dds"));
	MD.NormalMapIndex = 200;// CGameInstance::Get_Instance()->Add_Texture("DroneD", CTexture::Create(L"../bin/Models/Drone/Drone.dds"));
	m_CBBindingCom->Set_MaterialIndex(CGameInstance::Get_Instance()->Add_Material("ZetMat", MD));
	m_CBBindingCom->Set_TexCoordMatrix(XMMatrixIdentity());

	m_TransformCom->Set_Scale(10.f);

	m_TransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), 3.141592 * 0.5f);

	return S_OK;
}

void CZet::Tick(float fTimeDelta)
{
	if (m_GameInstance->Mouse_Down(2))
	{
		Set_StartPos_And_Move(SelectNum);
		SelectNum += 3;
		if (SelectNum >= 9)
			SelectNum = 0;
	}

	if (XMVectorGetX(m_TransformCom->Get_State(CTransform::STATE_POSITION)) >= 2048.f)
		isAlive = false;

	if (isAlive)
	{
		m_TransformCom->Go_Straight(fTimeDelta * 1000.f);
	}

}

void CZet::LateTick(float fTimeDelta)
{
	if (isAlive)
	{
		__super::LateTick(fTimeDelta);
		m_CBBindingCom->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);

	}

}

void CZet::Render()
{
	if (isAlive)
	{
		m_CBBindingCom->Set_On_Shader();
		m_VIBuffer->Render(0);
	}

}

void CZet::Free()
{
	Safe_Release(m_VIBuffer);
	Safe_Release(m_CBBindingCom);

	__super::Free();
}

CZet* CZet::Create()
{
	CZet* pInstance = new CZet;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CRenderObject* CZet::Clone(void* pArg)
{
	CZet* pInstance = new CZet(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}