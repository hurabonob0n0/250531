#include "Client_pch.h"
#include "BulletPath.h"
#include "GameInstance.h"
#include "Terrain.h"
#include "Tank.h"
#include "Network_Manager.h"

CBulletPath::CBulletPath() : CRenderObject()
{
}

CBulletPath::CBulletPath(CBulletPath& rhs) : CRenderObject(rhs)
{
}

HRESULT CBulletPath::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CBulletPath::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_BULLETPATH; // 바꿔야함.

	BulletPathstr* Info = (BulletPathstr*)pArg;
	OwnerTankIndex = Info->OwnerTankIndex;
	m_Dir = Info->Dir * 150.f;
	m_Pos1 = Info->Pos;
	m_Pos2 = Info->Pos;
	XMMATRIX mat = XMMatrixTranslation(XMVectorGetX(Info->Pos), XMVectorGetY(Info->Pos), XMVectorGetZ(Info->Pos));

	__super::Initialize(&mat);

	CVIBuffer_Geos::BASIC_SUBMESHES cylinder = CVIBuffer_Geos::BASIC_SUBMESHES::BS_CYLINDER;
	m_VIBuffer = (CVIBuffer_Geos*)m_GameInstance->Get_Component("VIBuffer_GeosCom", &cylinder);

	//m_VIBuffer = (CVIBuffer_Geos*)m_GameInstance->Get_Component("VIBuffer_GeosCom");
	m_Terrain = (CTerrain*)m_GameInstance->GetGameObject("Terrain", 0);
	return S_OK;
}


void CBulletPath::Tick(float fTimeDelta)
{
	//m_fDeltaTime += fTimeDelta;
	m_fDeltaTime += fTimeDelta;

	m_fYSpeed -= 6.8 * fTimeDelta;

	m_Pos2 += m_Dir * fTimeDelta + XMVectorSet(0.f, m_fYSpeed * fTimeDelta, 0.f, 0.f);



	if (m_fDeltaTime >= m_fAddBulletTime && BulletDatas.size() < 1000 && !isCollision)
	{
		BulletDatas.push_back(CreateBulletTrailInstance(m_Pos1, m_Pos2));
		for (int i = 0; i < BulletDatas.size(); ++i)
		{
			BulletDatas[i].ObjPad2 += m_fDeltaTime;
		}
		m_Pos1 = m_Pos2;
		m_fDeltaTime = 0.f;
	}
	else if (m_fDeltaTime >= m_fAddBulletTime && BulletDatas.size() < 1000 && isCollision)
	{
		for (int i = 0; i < BulletDatas.size(); ++i)
		{
			BulletDatas[i].ObjPad2 += m_fDeltaTime;
		}
		m_fDeltaTime = 0.f;
	}


	if (isCollision)
		m_fCollisionDeltatime += fTimeDelta;


	if (BulletDatas.size() >= 1000)
		isDead = true;
}

void CBulletPath::LateTick(float fTimeDelta){

	if (Network_Manager::GetInstance()->isConnected() && (m_GameInstance->GetLayerSize("Tank") > 1))
	{
		if (CheckCollisionWithTank()) {
			isCollision = true;
		}
	}

	if (CheckCollisionWithTerrain())
	{
		isCollision = true;
	}

	if (m_fCollisionDeltatime > 5.f)
		isDead = true;

	__super::LateTick(fTimeDelta);
}

void CBulletPath::Render()
{
	for (int i = 0; i < BulletDatas.size(); ++i)
	{
		m_GameInstance->Get_Current_FrameResource()->m_InstanceCB->CopyData(i, BulletDatas[i]);
	}
	GETCOMMANDLIST->SetGraphicsRootShaderResourceView(6, m_GameInstance->Get_Current_FrameResource()->m_InstanceCB->Resource()->GetGPUVirtualAddress());
	(CVIBuffer_Geos*)m_VIBuffer->Render((int)BulletDatas.size()); // 인스턴싱용으로 바꿔야함.
}

InstanceData CBulletPath::CreateBulletTrailInstance(const XMVECTOR& oldPos, const XMVECTOR& newPos)
{
	InstanceData data;

	// 1. Look 벡터 계산 (현재 위치 - 이전 위치)
	XMVECTOR look = XMVectorSubtract(newPos, oldPos);

	// 2. 궤적 조각의 길이(Scale) 계산
	float distance = XMVectorGetX(XMVector3Length(look));
	look = XMVector3Normalize(look); // Look 벡터 정규화

	// 3. Right, Up 벡터 계산
	XMVECTOR up, right;
	XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Look 벡터가 World Up 벡터와 거의 평행한 경우 (수직으로 쏘는 경우)
	if (XMVectorGetX(XMVector3Dot(look, worldUp)) > 0.999f || XMVectorGetX(XMVector3Dot(look, worldUp)) < -0.999f)
	{
		// 다른 기준 벡터(예: World Forward)를 사용
		XMVECTOR worldForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		right = XMVector3Normalize(XMVector3Cross(look, worldForward));
		up = XMVector3Normalize(XMVector3Cross(right, look));
	}
	else
	{
		right = XMVector3Normalize(XMVector3Cross(worldUp, look));
		up = XMVector3Normalize(XMVector3Cross(look, right));
	}

	// 4. World 행렬 생성
	XMMATRIX worldMatrix = XMMatrixIdentity();

	// 각 축 벡터를 행렬에 설정 (회전)
	worldMatrix.r[0] = right;
	worldMatrix.r[1] = up;
	worldMatrix.r[2] = look;
	worldMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	// 스케일 행렬 생성 (궤적 두께 0.1, 길이는 distance)
	// Z축으로 길어지도록 설정합니다. VIBuffer의 정육면체 모델이 Z축을 바라보도록 제작되어야 합니다.
	XMMATRIX scaleMatrix = XMMatrixScaling(0.1f, 0.1f, distance);

	// 위치 행렬 생성 (두 위치의 중간 지점)
	XMVECTOR midPoint = XMVectorScale(XMVectorAdd(oldPos, newPos), 0.5f);
	XMMATRIX translationMatrix = XMMatrixTranslationFromVector(midPoint);


	// 최종 World 행렬: Scale -> Rotate -> Translate 순서로 곱셈
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	worldMatrix = XMMatrixMultiply(XMMatrixRotationX(XMConvertToRadians(90.f)), worldMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, translationMatrix);

	// HLSL은 기본적으로 column-major 이므로, 행렬을 전치(Transpose)해서 저장합니다.
	XMStoreFloat4x4(&data.World, XMMatrixTranspose(worldMatrix));

	return data;
}

bool CBulletPath::CheckCollisionWithTerrain()
{
	float x = XMVectorGetX(m_Pos2);
	float z = XMVectorGetZ(m_Pos2);

	if (x < -2000.f || x > 2000.f || z < -2000.f || z > 2000.f)
		return true;

	float terrainY = m_Terrain->Get_Terrain_Heights(x, z);

	float bulletY = XMVectorGetY(m_Pos2);

	return (bulletY <= terrainY);
}

bool CBulletPath::CheckCollisionWithTank()
{

	int targetTankIndex = (OwnerTankIndex == 0) ? 1 : 0;

	// 상대 탱크 가져오기
	CTank* pTank = dynamic_cast<CTank*>(m_GameInstance->GetGameObject("Tank", targetTankIndex));
	if (!pTank) return false;

	const int tankCount = m_GameInstance->GetLayerSize("Tank");
	if (tankCount <= 1) return false;

	constexpr float R = 5.0f;
	constexpr float R2 = R * R;

	// 탱크 월드 행렬에서 위치 추출
	for (int i = 0; i < tankCount; ++i)
	{
		if (i == OwnerTankIndex && OwnerTankIndex >= 0)  // 내(발사자) 탱크 제외
			continue;

		CTank* pTank = dynamic_cast<CTank*>(m_GameInstance->GetGameObject("Tank", i));
		if (!pTank) continue;


		// 탱크 중심 위치
		XMMATRIX tankWorld;
		tankWorld = pTank->Get_WorldMatrix();
		XMFLOAT4X4 m; XMStoreFloat4x4(&m, tankWorld);
		XMVECTOR tankPos = XMVectorSet(m._41, m._42, m._43, 1.0f);

		// 총알 현재 위치와의 거리 제곱
		XMVECTOR d = XMVectorSubtract(m_Pos2, tankPos);
		float distSq = XMVectorGetX(XMVector3LengthSq(d));

		if (distSq <= R2)
		{
			return true; // 충돌!
		}
	}

	return false; // 충돌 없음

}

void CBulletPath::Free()
{
	Safe_Release(m_VIBuffer);

	__super::Free();
}

CBulletPath* CBulletPath::Create()
{
	CBulletPath* pInstance = new CBulletPath;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CRenderObject* CBulletPath::Clone(void* pArg)
{
	CBulletPath* pInstance = new CBulletPath(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
