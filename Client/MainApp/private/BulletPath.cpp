#include "Client_pch.h"
#include "BulletPath.h"
#include "GameInstance.h"
#include "Terrain.h"
#include "Tank.h"
#include "Network_Manager.h"
#include "FMOD_Manager.h"

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

	const bool wasCollision = isCollision;

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

	// 싱글플레이: 서버가 S_WEAPON_HIT 를 보내 주지 않으므로 처음 땅에 닿은 순간 직접 폭발시킨다
	// (멀티는 서버가 판정해서 보낸 위치로 ClientPacketHandler 가 폭발을 만든다)
	// m_Pos2 는 충돌 뒤에도 계속 움직이므로 충돌한 그 프레임에만 처리한다
	if (!wasCollision && isCollision && !Network_Manager::GetInstance()->isConnected())
	{
		const float x = XMVectorGetX(m_Pos2);
		const float z = XMVectorGetZ(m_Pos2);
		if (x > -2000.f && x < 2000.f && z > -2000.f && z < 2000.f)	// 맵 밖으로 나간 포탄은 터뜨리지 않는다
		{
			// 이번 프레임에 땅 밑으로 조금 파고든 위치라 지면 높이로 올린다
			Spawn_Explosion(XMVectorSet(x, m_Terrain->Get_Terrain_Heights(x, z), z, 1.f));
		}
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

/*----------------------------------------------------------------------------
	발사 / 폭발 연출 (싱글플레이와 네트워크 공용)
	점광원 색은 1 을 넘는다 (HDR). 톤매핑 전 HDR RT 에서 Bloom bright pass 에 걸려 번져 보인다
	밝기는 CLightMgr 가 Duration 동안 (1 - t)^2 로 줄인다
----------------------------------------------------------------------------*/
void CBulletPath::Add_MuzzleFlashLight(_fvector vPos)
{
	CLightMgr::POINTLIGHT_DESC desc;
	XMStoreFloat3(&desc.Position, vPos);
	desc.Color = XMFLOAT3(6.0f, 3.5f, 1.2f);	// 주황빛 섬광
	desc.Radius = 20.f;
	desc.Duration = 10.f;						// 짧게 번쩍
	CGameInstance::Get_Instance()->Add_PointLight(desc);
}

void CBulletPath::Add_ExplosionLight(_fvector vPos)
{
	CLightMgr::POINTLIGHT_DESC desc;
	// 지면에 딱 붙이면 바닥 노멀과 거의 평행해 N·L 이 작아진다 → 조금 띄워서 주변 땅을 비추게
	XMStoreFloat3(&desc.Position, XMVectorAdd(vPos, XMVectorSet(0.f, 3.f, 0.f, 0.f)));
	desc.Color = XMFLOAT3(10.0f, 5.0f, 1.5f);	// 머즐보다 크고 오래
	desc.Radius = 40.f;
	desc.Duration = 10.f;
	CGameInstance::Get_Instance()->Add_PointLight(desc);
}

// Handle_S_BULLET_ADD 와 같은 연출 + 머즐 플래시 점광원
void CBulletPath::Spawn_Shot(uint8 ownerTankIndex, _fvector vPos, _fvector vDir)
{
	CGameInstance* pGameInstance = CGameInstance::Get_Instance();

	CBulletPath::BulletPathstr bps;
	bps.OwnerTankIndex = ownerTankIndex;
	bps.Dir = vDir;
	bps.Pos = vPos;
	pGameInstance->AddObject("BulletPath", "BulletPath", &bps);

	const float PosX = XMVectorGetX(vPos);
	const float PosY = XMVectorGetY(vPos);
	const float PosZ = XMVectorGetZ(vPos);

	_matrix mat = XMMatrixTranslation(PosX, PosY + 0.5f, PosZ);
	pGameInstance->AddObject("SmokeEffect", "Effect", &mat);
	mat = XMMatrixTranslation(PosX + 0.1f, PosY + 0.6f, PosZ + 0.1f);
	pGameInstance->AddObject("SmokeEffect", "Effect", &mat);
	mat = XMMatrixTranslation(PosX - 0.1f, PosY + 0.6f, PosZ - 0.1f);
	pGameInstance->AddObject("SmokeEffect", "Effect", &mat);

	auto* FM = FMOD_Manager::Get_Instance();
	AudioVec3 p{ PosX, PosY, PosZ };
	AudioVec3 v{ 0, 0, 0 };
	FMOD::Channel* ch = nullptr;
	if (FM->Play3D_ReturnChannel("Tank_Shot", p, v, &ch, /*volume=*/0.4f, /*paused=*/false) && ch) {
		float pitch = 0.98f + (rand() / (float)RAND_MAX) * (1.02f - 0.98f);
		ch->setPitch(pitch);
	}

	Add_MuzzleFlashLight(vPos);
}

// Handle_S_WEAPON_HIT 와 같은 연출 + 폭발 점광원
void CBulletPath::Spawn_Explosion(_fvector vPos)
{
	_matrix Hit_Matrix = XMMatrixIdentity();
	Hit_Matrix.r[3] = XMVectorSetW(vPos, 1.f);
	CGameInstance::Get_Instance()->AddObject("Effect", "Effect", &Hit_Matrix);

	auto* FM = FMOD_Manager::Get_Instance();
	AudioVec3 p{ XMVectorGetX(vPos), XMVectorGetY(vPos), XMVectorGetZ(vPos) };
	AudioVec3 v{ 0, 0, 0 };
	FMOD::Channel* ch = nullptr;
	if (FM->Play3D_ReturnChannel("Explosion", p, v, &ch, /*volume=*/2.f, /*paused=*/false) && ch) {
		float pitch = 0.97f + (rand() / (float)RAND_MAX) * (1.03f - 0.97f);
		ch->setPitch(pitch);
	}

	Add_ExplosionLight(vPos);
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
