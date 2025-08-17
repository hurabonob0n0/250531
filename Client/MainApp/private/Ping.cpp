#include "Client_pch.h"
#include "Ping.h"
#include "GameInstance.h"
#include "Network_Manager.h"
#include "Terrain.h"

CPing::CPing() : CRenderObject()
{
}

CPing::CPing(CPing& rhs) : CRenderObject(rhs)
{
	m_MatIndex = rhs.m_MatIndex;
}

HRESULT CPing::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	MaterialData mat{};

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("PingTex", CTexture::Create(L"../bin/Models/FinalUI/Ping.dds"));

	m_MatIndex = CGameInstance::Get_Instance()->Add_Material("PingMat", mat);

	return S_OK;
}

HRESULT CPing::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_BLEND;

	__super::Initialize(pArg);

	m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);
	m_Terrain = (CTerrain*)m_GameInstance->GetGameObject("Terrain", 0);
	m_CameraFree = (CTransform*)m_GameInstance->Get_Object_Component("Camera", 0, "TransformCom");
	m_DroneCamera = (CTransform*)m_GameInstance->Get_Object_Component("Camera", 1, "TransformCom");

	Set_Position();

	m_CBBindingCom->Set_MaterialIndex(m_MatIndex);

	m_VIBuffer = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_TexCoordTransformCom->Identity();

	return S_OK;
}

void CPing::Tick(float fTimeDelta)
{

	m_TimeDelta += fTimeDelta;

}

void CPing::LateTick(float fTimeDelta)
{
	m_TransformCom->Identity();
	
	_vector Look;
	
	if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE)
		Look= m_DroneCamera->Get_State(CTransform::STATE_POSITION) - LayContactPos;
	else
		Look = m_CameraFree->Get_State(CTransform::STATE_POSITION) - LayContactPos;

	m_TransformCom->Adjust_Axis(XMVector4Normalize(-Look));
	m_TransformCom->Set_Scale(XMVectorGetX(0.08f * XMVector3Length(Look)));
	m_TransformCom->Set_State(CTransform::STATE_POSITION, LayContactPos);

	m_CBBindingCom->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);

	__super::LateTick(fTimeDelta);


	if (m_TimeDelta > 5.f)
		isDead = true;

}

void CPing::Render()
{
	m_RendererCom->ChangePSO("PingPSO");

	m_CBBindingCom->Set_On_Shader();

	m_VIBuffer->Render();

	m_RendererCom->ChangePSO("EffectPSO");
}

void CPing::Set_Position()
{
	_vector Pos, Look;
	Pos = m_DroneCamera->Get_State(CTransform::STATE_POSITION);
	Look = m_DroneCamera->Get_State(CTransform::STATE_LOOK);
	

	bool isUnder = false;
	LayContactPos = Pos;

	for (int i = 0; i < 1000; ++i)
	{
		LayContactPos += Look;
		float y = m_Terrain->Get_Terrain_Heights(XMVectorGetX(LayContactPos), XMVectorGetZ(LayContactPos));
		if (y > XMVectorGetY(LayContactPos))
		{
			isUnder = true;
			XMVectorSetY(LayContactPos, y);
			break;
		}
	}

	if (isUnder == false)
		isDead = true;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, LayContactPos);
}

_vector CPing::Get_Pos()
{
	return LayContactPos;
}

void CPing::Set_Position(float x, float y, float z)
{
	LayContactPos = XMVectorSet(x, y, z, 1.f);
}

void CPing::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CPing* CPing::Create()
{
	CPing* pInstance = new CPing;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CPing* CPing::Clone(void* pArg)
{
	CPing* pInstance = new CPing(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
