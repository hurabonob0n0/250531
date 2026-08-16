#include "Client_pch.h"
#include "Camera_Drone.h"
#include "GameInstance.h"
#include "Client_Globals.h"
#include "Terrain.h"
#include "Network_Manager.h"
#include "StateMgr.h"
#include "AirDrop.h"
#include "Ping.h"
#include "ClientPacketHandler.h"

CCamera_Drone::CCamera_Drone() : CCamera()
{
}

CCamera_Drone::CCamera_Drone(CCamera_Drone& rhs) : CCamera(rhs)
{
}

HRESULT CCamera_Drone::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CCamera_Drone::Initialize(void* pArg)
{
	__super::Initialize(pArg);
	m_PS = FPS;
	Set_RenderGroup(CRenderer::RG_PRIORITY);

	m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);

	m_CBBindingCom->Set_MaterialIndex(m_GameInstance->Add_Texture("SkyBox", CTexture::Create(L"../bin/Models/SkyBox/desertcube1024.dds"), CTextureMgr::TT_TEXTURECUBE));

	CVIBuffer_Geos::BASIC_SUBMESHES BS = CVIBuffer_Geos::BS_SPHERE;

	m_VIBuffer = (CVIBuffer_Geos*)m_GameInstance->Get_Component("VIBuffer_GeosCom", &BS);


	if (Network_Manager::GetInstance()->isConnected()) {
		m_DroneTransform = (CTransform*)m_GameInstance->Get_Object_Component("Drone", Network_Manager::GetInstance()->GetMyTankIndex(), "TransformCom");
		Safe_AddRef(m_DroneTransform);
	}
	else {
		m_DroneTransform = (CTransform*)m_GameInstance->Get_Object_Component("Drone", 0, "TransformCom");
		Safe_AddRef(m_DroneTransform);

	}
	m_Distance_TPS = 20.f;

	m_fYRot_TPS = 0.f;

	m_fXRot_TPS = 0.f;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, 0.f, -15.f, 1.f));

	m_Terrain = (CTerrain*)m_GameInstance->GetGameObject("Terrain", 0);

	//if (Network_Manager::GetInstance()->ImPosu)
	//	m_PS = FPS;
	return S_OK;
}

void CCamera_Drone::Tick(float fTimeDelta)
{
	if (m_GameInstance->Mouse_Down(2))
	{

		if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE) {
			m_GameInstance->AddObject("Ping", "Ping", nullptr);
			int index = m_GameInstance->GetLayerSize("Ping") - 1;

			CPing* pingObj = dynamic_cast<CPing*>(m_GameInstance->GetGameObject("Ping", index));
			if (!pingObj) return;

			XMVECTOR Temp = pingObj->Get_Pos();

			float x = XMVectorGetX(Temp);
			float y = XMVectorGetY(Temp);
			float z = XMVectorGetZ(Temp);

			if (Network_Manager::GetInstance()->isConnected()) {
				auto sendBuffer = ClientPacketHandler::Make_C_PING(x, y, z);
				Network_Manager::GetInstance()->Send(sendBuffer);
			}
		}

	}

	//TODO 여기 키 변경 Y + Drone의 3인칭은 버리기
	//
	if (m_GameInstance->Key_Down('G'))
	{
		isRotation = !isRotation;

		if (isRotation)
		{
			m_fYRot_TPS = m_fXRot_TPS = 0.f;
			m_TransformCom->Identity();
			m_TransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&LastPos));
		}
	}

	if (m_GameInstance->Key_Down('Y')) {
		if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_POSIN) {
			SetDroneRender(true);
			Network_Manager::GetInstance()->MyControlTarget = CONTROL_DRONE;
			((CUI_AirDrop*)(CGameInstance::Get_Instance()->GetGameObject("UI", UI_AIRDROP)))->set_render_off();
			CStateMgr::Set_GameMode(GM_Drone);
			m_PS = FPS;
		}
		else if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE) {
			SetDroneRender(false);
			Network_Manager::GetInstance()->MyControlTarget = CONTROL_POSIN;
			CStateMgr::Set_GameMode(GM_FPS);
			m_PS = FPS;
		}
	}

	if (isRotation == false)
	{
		XMStoreFloat4(&LastPos, m_TransformCom->Get_State(CTransform::STATE_POSITION));
	}


	//__super::Tick(fTimeDelta);
	if (m_GameInstance->Key_Down(VK_PAUSE))
		m_isPaused = !m_isPaused;

	if (!m_isPaused) {



		if (Network_Manager::GetInstance()->MyControlTarget != CONTROL_DRONE) {
			if (m_GameInstance->Mouse_Down(1))
			{
				if (m_PS == FPS)
					m_PS = TPS;

				else if (m_PS == TPS)
					m_PS = FPS;
			}
		}
		if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE) {
			switch (m_PS)
			{
			case Client::CCamera_Drone::FPS:
				Tick_For_FPS(fTimeDelta);
				break;
			case Client::CCamera_Drone::TPS:
				Tick_For_TPS(fTimeDelta);
				break;

			default:
				break;
			}
		}
	}

}

void CCamera_Drone::LateTick(float fTimeDelta)
{
	m_RendererCom->AddtoRenderObjects(m_RG, this);

	XMMATRIX world = XMMatrixScaling(5000.f, 5000.f, 5000.f);
	_matrix textransform = m_TexCoordTransformCom->Get_WorldMatrix();

	_float3 Pos;

	XMStoreFloat3(&Pos, m_TransformCom->Get_State(CTransform::STATE_POSITION));

	/*float terrainY = m_Terrain->Get_Terrain_Heights(Pos.x, Pos.z);

	if (Pos.y <= terrainY)
		Pos.y = terrainY + 1.f;*/

	m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(Pos.x, Pos.y, Pos.z, 1.f));

	m_CBBindingCom->Set_CBIndex();
	m_CBBindingCom->Set_WorldMatrix(world);
	m_CBBindingCom->Set_TexCoordMatrix(textransform);
	m_CBBindingCom->Update_CBView();

	if (isDroneRender)
		__super::LateTick(fTimeDelta);
}

void CCamera_Drone::Render()
{
	if (isDroneRender)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(m_GameInstance->Get_SRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
		skyTexDescriptor.Offset(m_CBBindingCom->Get_MaterialIndex(), m_GameInstance->Get_CBVUAVSRVHeapSize());
		GETCOMMANDLIST->SetGraphicsRootDescriptorTable(3, skyTexDescriptor);

		m_CBBindingCom->Set_On_Shader();

		m_VIBuffer->Render();
	}
}

void CCamera_Drone::Tick_For_TPS(float fTimeDelta)
{
	m_fYRot_TPS += m_GameInstance->Get_Mouse_XDelta() * 0.005f;
	m_fXRot_TPS += m_GameInstance->Get_Mouse_YDelta() * 0.005f;
	m_Distance_TPS -= (float)m_GameInstance->Get_Mouse_Scroll() * 0.005f;

	m_fXRot_TPS = max(-85.f, min(85.f, m_fXRot_TPS));

	_float4x4 mat;
	XMStoreFloat4x4(&mat, XMMatrixIdentity());

	m_TransformCom->Set_WorldMatrix(mat);

	_vector TankPos = m_DroneTransform->Get_State(CTransform::STATE_POSITION);
	_vector myPos = XMVectorSet(0.f, 0.f, -m_Distance_TPS, 1.f);
	myPos += TankPos;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, myPos);

	m_TransformCom->Orbit_For_TPS(TankPos, m_fYRot_TPS, m_fXRot_TPS);
}

void CCamera_Drone::Tick_For_FPS(float fTimeDelta)
{
	m_fYRot_TPS += m_GameInstance->Get_Mouse_XDelta() * 0.005f;
	m_fXRot_TPS += m_GameInstance->Get_Mouse_YDelta() * 0.005f;
	m_Distance_TPS += (float)m_GameInstance->Get_Mouse_Scroll() * 0.005f;

	m_fXRot_TPS = max(-85.f, min(85.f, m_fXRot_TPS));

	_float4x4 mat;
	XMStoreFloat4x4(&mat, XMMatrixIdentity());

	m_TransformCom->Set_WorldMatrix(mat);

	_vector TankPos = m_DroneTransform->Get_State(CTransform::STATE_POSITION);
	_vector myPos = XMVectorSet(0.f, 0.f, -3.f, 1.f);
	myPos += TankPos;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, myPos);

	m_TransformCom->Orbit_For_FPS(TankPos, m_fYRot_TPS, m_fXRot_TPS);

	myPos = m_TransformCom->Get_State(CTransform::STATE_POSITION);
	float y = XMVectorGetY(myPos) + 5.f;
	myPos = XMVectorSetY(myPos, y);
	m_TransformCom->Set_State(CTransform::STATE_POSITION, myPos);

}

void CCamera_Drone::Free()
{
	Safe_Release(m_VIBuffer);
	Safe_Release(m_DroneTransform);
	Safe_Release(m_CBBindingCom);
	__super::Free();
}

CCamera_Drone* CCamera_Drone::Create()
{
	CCamera_Drone* pInstance = new CCamera_Drone;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CCamera_Drone* CCamera_Drone::Clone(void* pArg)
{
	CCamera_Drone* pInstance = new CCamera_Drone(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
