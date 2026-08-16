#include "Client_pch.h"
#include "Camera_Free.h"
#include "GameInstance.h"
#include "Client_Globals.h"
#include "Terrain.h"
#include "Network_Manager.h"
#include "StateMgr.h"
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

	m_Distance_FPS = 50.f;

	m_fXRot_FPS = 0.f;

	m_fYRot_FPS = 0.f;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, 0.f, -15.f, 1.f));

	m_Terrain = (CTerrain*)m_GameInstance->GetGameObject("Terrain", 0);

	if (Network_Manager::GetInstance()->MyPosMode == POS_POSU)
		m_PS = FPS;

	return S_OK;
}

void CCamera_Free::Tick(float fTimeDelta)
{
	/*if (m_GameInstance->Mouse_Down(2))
	{
		_uint CamIndex = 0;
		m_GameInstance->AddObject("Ping", "Ping", &CamIndex);
	}*/

	//__super::Tick(fTimeDelta);
	if (m_GameInstance->Key_Down(VK_PAUSE))
		m_isPaused = !m_isPaused;

	if (!m_isPaused) {
		if (Network_Manager::GetInstance()->MyPosMode == POS_MASTER) {

			if (m_GameInstance->Mouse_Down(1))
			{

				if (Network_Manager::GetInstance()->MyControlTarget != CONTROL_DRONE) {
					if (m_PS == FPS) {
						Network_Manager::GetInstance()->MyControlTarget = CONTROL_TANK;
						CStateMgr::Set_GameMode(GM_TPS);
						m_PS = TPS;

					}

					else if (m_PS == TPS) {
						m_PS = FPS;
						Network_Manager::GetInstance()->MyControlTarget = CONTROL_POSIN;
						CStateMgr::Set_GameMode(GM_FPS);
					}
				}
			}
		}

		switch (m_PS)
		{
		case Client::CCamera_Free::FPS:
			if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_POSIN)
				Tick_For_FPS(fTimeDelta);
			break;
		case Client::CCamera_Free::TPS:
			Tick_For_TPS(fTimeDelta);
			break;

		default:
			break;
		}

	}

	ApplyCameraShake(fTimeDelta);

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

	PassCBSetting();

	//__super::LateTick(fTimeDelta);
}

void CCamera_Free::Render()
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE skyTexDescriptor(m_GameInstance->Get_SRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
	skyTexDescriptor.Offset(m_CBBindingCom->Get_MaterialIndex(), m_GameInstance->Get_CBVUAVSRVHeapSize());
	GETCOMMANDLIST->SetGraphicsRootDescriptorTable(3, skyTexDescriptor);

	m_CBBindingCom->Set_On_Shader();

	m_VIBuffer->Render();
}

void CCamera_Free::PassCBSetting()
{

	XMMATRIX proj;
	if (m_PS == TPS)
	{
		 proj = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(60.0f), // Field of View (radian 단위)
			1.7777,               // Aspect ratio = width / height
			1.f,                     // Near clipping plane
			10000.f                       // Far clipping plane
		);
	}
	else if (m_PS == FPS)
	{
		proj = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(m_Distance_FPS), // Field of View (radian 단위)
			1.7777,               // Aspect ratio = width / height
			0.01f,                     // Near clipping plane
			10000.f                       // Far clipping plane
		);
	}

	PassConstants pc{};

	XMMATRIX view = m_TransformCom->Get_WorldMatrix_Inverse();//XMLoadFloat4x4(&mView);
	//XMMATRIX view = m_TransformCom->Get_WorldMatrix();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(nullptr, view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&pc.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&pc.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&pc.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&pc.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&pc.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&pc.InvViewProj, XMMatrixTranspose(invViewProj));
	//XMStoreFloat4x4(&pc.ShadowTransform, XMMatrixTranspose(m_GameInstance->m_ShadowMap->S));
	XMStoreFloat4x4(&pc.ShadowTransform, XMMatrixTranspose(m_GameInstance->m_ShadowMap->S));
	XMStoreFloat3(&pc.EyePosW, m_TransformCom->Get_State(CTransform::STATE_POSITION));
	pc.RenderTargetSize = XMFLOAT2((float)1920, (float)1080);
	pc.InvRenderTargetSize = XMFLOAT2(1.0f / 1920, 1.0f / 1080);
	pc.NearZ = 1.0f;
	pc.FarZ = 10000.0f;
	pc.AmbientLight = { 0.25f, 0.25f, 0.25f, 1.0f };
	pc.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	pc.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	pc.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	pc.Lights[1].Strength = { 0.1f, 0.1f, 0.1f };
	pc.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	pc.Lights[2].Strength = { 0.1f, 0.1f, 0.1f };

	//XMStoreFloat4x4(&pc.View, XMMatrixTranspose(view));
	//XMStoreFloat4x4(&pc.InvView, XMMatrixTranspose(invView));
	//XMStoreFloat4x4(&pc.Proj, XMMatrixTranspose(proj));
	//XMStoreFloat4x4(&pc.ViewProj, XMMatrixTranspose(viewProj));
	//XMVECTOR vec = m_TransformCom->Get_State(CTransform::STATE_POSITION);
	//XMStoreFloat3(&pc.EyePosW, m_TransformCom->Get_State(CTransform::STATE_POSITION));
	//XMStoreFloat4x4(&pc.ShadowTransform, XMMatrixTranspose(m_GameInstance->m_ShadowMap->S));
	////XMStoreFloat4x4(&pc.ShadowTransform, m_GameInstance->m_ShadowMap->S);
	//pc.AmbientLight = { 0.25f, 0.25f, 0.25f, 1.0f };
	//pc.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	//pc.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	//pc.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	//pc.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	//pc.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	//pc.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	/* 지형이 이 값으로 청크를 절두체 컬링하고 LOD를 고른다. */
	m_GameInstance->Set_CameraViewProj(viewProj, m_TransformCom->Get_State(CTransform::STATE_POSITION));

	auto currPassCB = m_GameInstance->Get_Current_FrameResource()->m_PassCB;
	currPassCB->CopyData(0, pc);
}

void CCamera_Free::Tick_For_TPS(float fTimeDelta)
{
	m_fYRot_TPS += m_GameInstance->Get_Mouse_XDelta() * 0.005f;
	m_fXRot_TPS += m_GameInstance->Get_Mouse_YDelta() * 0.005f;
	//m_Distance_TPS -= (float)m_GameInstance->Get_Mouse_Scroll() * 0.005f;

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
	m_fYRot_FPS += m_GameInstance->Get_Mouse_XDelta() * 0.1f;
	m_fXRot_FPS += m_GameInstance->Get_Mouse_YDelta() * 0.1f;
	m_Distance_FPS -= (float)m_GameInstance->Get_Mouse_Scroll() * 0.005f;

	m_fXRot_FPS = max(-85.f, min(85.f, m_fXRot_FPS));

	m_TransformCom->Identity();

	_vector TankLook = XMVector4Normalize(m_TankTransform->Get_State(CTransform::STATE_LOOK));
	_vector TankRight = m_TankTransform->Get_State(CTransform::STATE_RIGHT);
	_vector TankUp = m_TankTransform->Get_State(CTransform::STATE_UP);
	_vector TankPos = m_TankTransform->Get_State(CTransform::STATE_POSITION);

	_vector BasePos = TankUp * 2.4f;// +TankLook * 1.4f;
	// 방향 벡터의 각 성분 가져오기
	float x = XMVectorGetX(TankLook);
	float y = XMVectorGetY(TankLook);
	float z = XMVectorGetZ(TankLook);

	// X축 회전값 (Pitch) 계산
	// pitch = asin(-y) 또는 asin(y) (좌표계에 따라 부호가 다를 수 있음)
	float pitch = asinf(y);

	// Y축 회전값 (Yaw) 계산
	float yaw = atan2f(x, z);
	_vector BaseLook = TankLook;//XMVector4Transform( XMVectorSet(0.f, 0.f, 1.f, 0.f),XMMatrixRotationX(pitch));

	_matrix matYaw = XMMatrixRotationAxis(TankUp,
		XMConvertToRadians(m_fYRot_FPS) + (3.141592 * 2 - yaw));

	_matrix matPitch = XMMatrixRotationAxis(TankRight,
		XMConvertToRadians(m_fXRot_FPS));

	// 4. 최종 회전 행렬 계산 (중요: Pitch를 먼저 곱하고 Yaw를 곱합니다)
	// 로컬 축 기준의 Pitch 변환이 먼저 적용되고, 그 결과가 Yaw 변환됩니다.
	_matrix matTotalRotation = matPitch * matYaw;

	BaseLook = XMVector3TransformCoord(BaseLook, matYaw);

	BasePos = BasePos + 1.55f * BaseLook;
	BasePos = TankPos + BasePos;
	XMVectorSetW(BasePos, 1.f);

	m_TransformCom->Set_State(CTransform::STATE_POSITION, BasePos);

	_vector vDir = XMVector3TransformNormal(TankLook, matTotalRotation);
	/*_vector vDir = XMVector3TransformNormal(XMVectorSet(0.f,0.f,1.f,0.f),
		XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_fXRot_FPS) * 1.f, XMConvertToRadians(m_fYRot_FPS),0.f));*/
		/*BaseLook = XMVector4Transform(BaseLook, RotationMatrix);*/

		//m_TransformCom->Look_At(BasePos + BaseLook);
	m_TransformCom->Adjust_Axis(XMVector4Normalize(vDir));

	m_TransformCom->Go_Left(0.1f);

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

void CCamera_Free::StartShake(float duration, float amplitude, float frequency)
{
	if (m_IsShaking) return; // 이미 진행 중이면 무시

	m_IsShaking = true;
	m_ShakeTime = 0.f;
	m_ShakeDur = duration;
	m_ShakeAmp = amplitude;
	m_ShakeFreq = frequency;

	// 프레임마다 동일 결과 방지용 위상 시드(간단히 난수 대용)
	// (진짜 난수 쓰고 싶으면 std::mt19937 등 사용)
	m_ShakeSeed += 37.123f;
}

void CCamera_Free::ApplyCameraShake(float dt)
{
	if (!m_IsShaking)
		return;

	m_ShakeTime += dt;
	float t = m_ShakeTime / m_ShakeDur;
	if (t >= 1.f)
	{
		// 종료
		m_IsShaking = false;
		m_ShakeOfs = { 0,0,0 };
		return;
	}

	// 부드러운 감쇠(끝으로 갈수록 줄어듦)
	// (지수 감쇠 + (1-t) 가중, 취향에 맞게 조절)
	float decay = expf(-3.5f * t) * (1.f - t);

	// 간단한 2축/3축 신호(위상 분리)
	float w = m_ShakeFreq * (m_ShakeTime + m_ShakeSeed);
	float ox = sinf(w) * m_ShakeAmp * decay;
	float oy = cosf(w * 1.2f + 1.73f) * m_ShakeAmp * 0.6f * decay; // 수직은 살짝 약하게
	float oz = sinf(w * 0.8f + 3.11f) * m_ShakeAmp * 0.4f * decay; // 전후 흔들림은 더 약하게

	m_ShakeOfs = { ox, oy, oz };

	// 카메라-공간 오프셋을 월드 위치에 적용
	using namespace DirectX;
	XMVECTOR pos = m_TransformCom->Get_State(CTransform::STATE_POSITION);
	XMVECTOR right = XMVector3Normalize(m_TransformCom->Get_State(CTransform::STATE_RIGHT));
	XMVECTOR up = XMVector3Normalize(m_TransformCom->Get_State(CTransform::STATE_UP));
	XMVECTOR look = XMVector3Normalize(m_TransformCom->Get_State(CTransform::STATE_LOOK));

	XMVECTOR ofs =
		right * m_ShakeOfs.x +
		up * m_ShakeOfs.y +
		look * m_ShakeOfs.z;

	m_TransformCom->Set_State(CTransform::STATE_POSITION, pos + ofs);

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
