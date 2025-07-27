#include "Client_pch.h"
#include "Camera_Free.h"
#include "GameInstance.h"
#include "Client_Globals.h"
#include "Terrain.h"

CCamera_Free::CCamera_Free() : CCamera()
{
}

CCamera_Free::CCamera_Free(CCamera_Free& rhs) : CCamera(rhs)
{
}

HRESULT CCamera_Free::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
	__super::Initialize(pArg);

	Set_RenderGroup(CRenderer::RG_PRIORITY);

	m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);

	m_CBBindingCom->Set_MaterialIndex(m_GameInstance->Add_Texture("SkyBox", CTexture::Create(L"../bin/Models/SkyBox/desertcube1024.dds"), CTextureMgr::TT_TEXTURECUBE));

	CVIBuffer_Geos::BASIC_SUBMESHES BS = CVIBuffer_Geos::BS_SPHERE;

	m_VIBuffer = (CVIBuffer_Geos*)m_GameInstance->Get_Component("VIBuffer_GeosCom", &BS);


	if (Network_Manager::GetInstance()->isConnected()) {
		m_TankTransform = (CTransform*)m_GameInstance->Get_Object_Component("Tank", Network_Manager::GetInstance()->GetMyTankIndex(), "TransformCom");
		Safe_AddRef(m_TankTransform);

		m_Tank = (CTank*)m_GameInstance->GetGameObject("Tank", Network_Manager::GetInstance()->GetMyTankIndex());
		Safe_AddRef(m_Tank);
	}
	else {
		m_TankTransform = (CTransform*)m_GameInstance->Get_Object_Component("Tank", 0, "TransformCom");
		Safe_AddRef(m_TankTransform);

		m_Tank = (CTank*)m_GameInstance->GetGameObject("Tank", 0);
		Safe_AddRef(m_Tank);

	}
	m_Distance_TPS = 20.f;

	m_fYRot_TPS = 0.f;

	m_fXRot_TPS = 0.f;

	m_Distance_FPS = 10.f;

	m_fXRot_FPS = 0.f;

	m_fYRot_FPS = 0.f;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, 0.f, -15.f, 1.f));

	m_Terrain = (CTerrain*)m_GameInstance->GetGameObject("Terrain", 0);

	if (Network_Manager::GetInstance()->ImPosu)
		m_PS = FPS;
	return S_OK;
}

void CCamera_Free::Tick(float fTimeDelta)
{


	//__super::Tick(fTimeDelta);
	if (m_GameInstance->Key_Down(VK_PAUSE))
		m_isPaused = !m_isPaused;

	if (!m_isPaused) {
		if (m_GameInstance->Mouse_Down(1))
		{
			if (m_PS == FPS)
				m_PS = TPS;

			else if (m_PS == TPS)
				m_PS = FPS;
		}

		switch (m_PS)
		{
		case Client::CCamera_Free::FPS:
			Tick_For_FPS(fTimeDelta);
			break;
		case Client::CCamera_Free::TPS:
			Tick_For_TPS(fTimeDelta);
			break;

		default:
			break;
		}

	}

}

void CCamera_Free::LateTick(float fTimeDelta)
{
	m_RendererCom->AddtoRenderObjects(m_RG, this);

	XMMATRIX world = XMMatrixScaling(5000.f, 5000.f, 5000.f);
	_matrix textransform = m_TexCoordTransformCom->Get_WorldMatrix();

	_float3 Pos;

	XMStoreFloat3(&Pos, m_TransformCom->Get_State(CTransform::STATE_POSITION));

	float terrainY = m_Terrain->Get_Terrain_Heights(Pos.x, Pos.z);

	if (Pos.y <= terrainY)
		Pos.y = terrainY + 1.f;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(Pos.x, Pos.y, Pos.z, 1.f));

	m_CBBindingCom->Set_CBIndex();
	m_CBBindingCom->Set_WorldMatrix(world);
	m_CBBindingCom->Set_TexCoordMatrix(textransform);
	m_CBBindingCom->Update_CBView();

	__super::LateTick(fTimeDelta);
}

void CCamera_Free::Render()
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(m_GameInstance->Get_SRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(m_CBBindingCom->Get_MaterialIndex(), m_GameInstance->Get_CBVUAVSRVHeapSize());
	GETCOMMANDLIST->SetGraphicsRootDescriptorTable(3, skyTexDescriptor);

	m_CBBindingCom->Set_On_Shader();

	m_VIBuffer->Render();
}

void CCamera_Free::Tick_For_TPS(float fTimeDelta)
{
	m_fYRot_TPS += m_GameInstance->Get_Mouse_XDelta() * 0.005f;
	m_fXRot_TPS += m_GameInstance->Get_Mouse_YDelta() * 0.005f;
	m_Distance_TPS -= (float)m_GameInstance->Get_Mouse_Scroll() * 0.005f;

	m_fXRot_TPS = max(-85.f, min(85.f, m_fXRot_TPS));

	_float4x4 mat;
	XMStoreFloat4x4(&mat, XMMatrixIdentity());

	m_TransformCom->Set_WorldMatrix(mat);

	_vector TankPos = m_TankTransform->Get_State(CTransform::STATE_POSITION);
	_vector myPos = XMVectorSet(0.f, 0.f, -m_Distance_TPS, 1.f);
	myPos += TankPos;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, myPos);

	m_TransformCom->Orbit_For_TPS(TankPos, m_fYRot_TPS, m_fXRot_TPS);
}

void CCamera_Free::Tick_For_FPS(float fTimeDelta)
{
	m_fYRot_FPS += m_GameInstance->Get_Mouse_XDelta()* 0.1f;
	m_fXRot_FPS += m_GameInstance->Get_Mouse_YDelta() * 0.1f;
	m_Distance_FPS += (float)m_GameInstance->Get_Mouse_Scroll() * 0.005f;

	m_fXRot_FPS = max(-85.f, min(85.f, m_fXRot_FPS));

	_float4x4 mat;
	XMStoreFloat4x4(&mat, XMMatrixIdentity());

	m_TransformCom->Set_WorldMatrix(mat);

	_vector TankPos = m_TankTransform->Get_State(CTransform::STATE_POSITION);
	_vector myPos = XMVectorSet(0.f, 0.f, -m_Distance_FPS, 1.f);
	myPos += TankPos;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, myPos);

	m_TransformCom->Orbit_For_FPS(TankPos,XMConvertToRadians( m_fYRot_FPS), XMConvertToRadians(m_fXRot_FPS));

	myPos = m_TransformCom->Get_State(CTransform::STATE_POSITION);
	float y = XMVectorGetY(myPos) + 5.f;
	myPos = XMVectorSetY(myPos, y);
	m_TransformCom->Set_State(CTransform::STATE_POSITION, myPos);

	//_matrix matforBox = XMMatrixIdentity();

	/*XMStoreFloat3(&vRight, XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f,1.f,0.f,0.f), worldforward)));

	XMStoreFloat3(&vUp, XMVector3Normalize(XMVector3Cross(worldforward, XMLoadFloat3(&vRight))));
	matforBox.r[1] = vUp;*/
	//matforBox.r[2] = m_TransformCom->Get_State(CTransform::STATE_LOOK);
	//matforBox.r[3] = m_TransformCom->Get_State(CTransform::STATE_POSITION);




	//if (m_GameInstance->Mouse_Down(0))
	//	m_GameInstance->AddObject("DefaultObject", "BoxObj", &matforBox);

	_vector camPos = m_TransformCom->Get_State(CTransform::STATE_POSITION);
	_vector camLook = XMVector3Normalize(m_TransformCom->Get_State(CTransform::STATE_LOOK));

	// 총알 스폰 위치: 카메라 방향 기준 정면 6.f 앞
	_vector muzzlePos = camPos + camLook * 6.f;

	// 총알용 행렬 구성
	_matrix matforBox = XMMatrixIdentity();
	matforBox.r[2] = camLook;       // Look 방향
	matforBox.r[3] = muzzlePos;     // 위치

	// 발사 정보 전달

	m_Tank->Set_ShotMatrix(matforBox);
	m_Tank->Set_PotapRotation(m_fYRot_FPS);
	m_Tank->Set_PoSinpRotation(m_fXRot_FPS);

}

void CCamera_Free::Free()
{
	Safe_Release(m_VIBuffer);
	Safe_Release(m_TankTransform);
	Safe_Release(m_Tank);
	Safe_Release(m_CBBindingCom);
	__super::Free();
}

CCamera_Free* CCamera_Free::Create()
{
	CCamera_Free* pInstance = new CCamera_Free;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CCamera_Free* CCamera_Free::Clone(void* pArg)
{
	CCamera_Free* pInstance = new CCamera_Free(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
