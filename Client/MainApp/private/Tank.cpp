#include "Client_pch.h"
#include "Tank.h"
#include "Client_Defines.h"
#include "GameInstance.h"
#include "ClientPacketHandler.h"
#include "ServiceManager.h"
#include "Network_Manager.h"
#include "UIReloading.h"
#include "UISelectPos.h"
#include "BulletPath.h"
#include "Drone.h"
#include "AirDrop.h"
#include "FMOD_Manager.h"
#include "Camera_Free.h"



static inline AudioVec3 ToAudio(const XMFLOAT3& v) { return AudioVec3(v.x, v.y, v.z); }
static inline AudioVec3 ToAudio(const XMVECTOR& v) { XMFLOAT3 t; XMStoreFloat3(&t, v); return ToAudio(t); }



CTank::CTank() : CRenderObject()
{
	_isSpawn = true;
}

CTank::CTank(CTank& rhs) : CRenderObject(rhs)
{
	_isSpawn = true;
}

void CTank::Set_Team(int Team)
{

	m_VIBuffer->Set_Team(Team);
}

HRESULT CTank::Initialize_Prototype()
{


	__super::Initialize_Prototype();

	matindex = CGameInstance::Get_Instance()->Get_Mat_Size();

	MaterialData mat{};
	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Glacis_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glacis_Plate_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Glacis_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glacis_Plate_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Glacis", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Glass_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glass_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Glass_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glass_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Glass", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Fence_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Fences_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Fence_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Fences_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Fence", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Wheels_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Wheels_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Wheels_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Wheels_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Wheels", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Turret_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Turret_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Turret_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Turret_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Turret", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_W_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_W_Base_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_W_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_W_Base_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_W", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Sprocket_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Sprocket_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Sprocket_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Sprocket_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Sprocket", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Tracks_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Tracks_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Tracks_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Tracks_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Tracks", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_M250_D", CTexture::Create(L"../bin/Models/TankDDS/M2-50_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_M250_N", CTexture::Create(L"../bin/Models/TankDDS/M2-50_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_M250", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_M240P_D", CTexture::Create(L"../bin/Models/TankDDS/M240P_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_M240P_N", CTexture::Create(L"../bin/Models/TankDDS/M240P_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_M240P", mat);

	return S_OK;
}

HRESULT CTank::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_NONBLEND;

	__super::Initialize(pArg);

	//CModel* pModel = (CModel*)m_GameInstance->Get_Component("ModelCom");
	m_VIBuffer = (CModel*)m_GameInstance->Get_Component("ModelCom");

	m_VIBuffer->Set_MatOffsets(matindex);

	m_pPhysicsEngine = MyPhysicsEngine::CMyPhysicsEngine::Get_Instance();

	Set_Team(1);
	//set_Spawn(false);
	is_RespawnArea_choiced = false;
	Initialize_For_PosinQuad();

	return S_OK;
}

void CTank::Initialize_For_PosinQuad()
{
	m_CBBindingQuad = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);
	MaterialData mat{};

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Posin", CTexture::Create(L"../bin/Models/Posin/cross.dds"));

	m_CBBindingQuad->Set_MaterialIndex(m_GameInstance->Add_Material("PosinMat", mat));

	m_VIBufferQuad = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_QuadWorldTransform = (CTransform*)m_GameInstance->Get_Component("TransformCom", nullptr);

	m_QuadTexTransform = (CTransform*)m_GameInstance->Get_Component("TransformCom", nullptr);
}

void CTank::Tick(float fTimeDelta)
{
	if (_myPlayer) {
		UpdateAirDropCooldown(fTimeDelta);
		_shootTimer += fTimeDelta;

		if (!_isSpawn)
		{
			_respawnTimer += fTimeDelta;
			if (_respawnTimer >= 5.f)
			{
				if(Network_Manager::GetInstance()->MyPosMode != POS_POSU)
					setRespawn();
			}
			return;
		}


		switch (Network_Manager::GetInstance()->MyPosMode) {
		case POS_MASTER: {
			
			Master_Pos_KeyInput();

		}
			 break;
		case POS_DRIVER: {

			Driver_Pos_KeyInput();

		}
			break;
		case POS_POSU: {

			POSU_Pos_KeyInput();

		}
			break;
		default:
			break;

		}

	}
	
}


void CTank::LateTick(float fTimeDelta)
{

	__super::LateTick(fTimeDelta);
	

	if (_myPlayer) {

	
		switch (Network_Manager::GetInstance()->MyPosMode) {
		case POS_MASTER: {
			RotPotap_And_Posin(fTimeDelta);
			Set_Tank_Element_from_Engine();
			Tick_For_Posin_Image(fTimeDelta);
			ErrorRespawn();

			if (Network_Manager::GetInstance()->MyControlTarget != CONTROL_DRONE) {
				if (m_GameInstance->Mouse_Down(1)) {
					m_isFPS = !m_isFPS;
				}
			}
			if (Network_Manager::GetInstance()->isConnected())
				SendMyStateToServer();

		}
					   break;
		case POS_DRIVER: {
			Set_Tank_Element_from_Engine();
			ErrorRespawn();
			if (Network_Manager::GetInstance()->isConnected())
				SendPosData();

		}
					   break;
		case POS_POSU: {

			Set_Tank_Element_from_ServerData();
			Tick_For_Posin_Image(fTimeDelta);
			RotPotap_And_Posin(fTimeDelta);
			if(Network_Manager::GetInstance()->isConnected())
				SendPosinData();

		}
					 break;
		default:
			break;

		}
	}
	else {

		Set_Tank_Element_from_ServerData();

	}



	UpdateAudio(fTimeDelta);

}



void CTank::Render()
{

	if (_myPlayer) {

		switch (Network_Manager::GetInstance()->MyPosMode) {
		case POS_MASTER:
		{
			if (Network_Manager::GetInstance()->MyControlTarget==CONTROL_POSIN) {

				m_RendererCom->ChangePSO("UIPSO");
				Render_For_Posin_Image();
				m_RendererCom->ChangePSO("DefaultPSO");
				m_VIBuffer->Render(50);
			}
			else
			{
				for (int i = 0; i < 55; ++i)
					m_VIBuffer->Render(i);
			}


		}
			break;
		case POS_DRIVER:
		{
			for (int i = 0; i < 55; ++i)
				m_VIBuffer->Render(i);

		}
			break;
		case POS_POSU:
		{
			if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_POSIN) {

				m_RendererCom->ChangePSO("UIPSO");
				Render_For_Posin_Image();
				m_RendererCom->ChangePSO("DefaultPSO");
			}
			else {
				for (int i = 0; i < 55; ++i)
					m_VIBuffer->Render(i);
			}
		}
			break;
		default:
			break;
		}

	}
	else {

		for (int i = 0; i < 55; ++i)
			m_VIBuffer->Render(i);
	}

}

void CTank::Set_PotapRotation(float fDegree)
{
	m_fCamPotapRot = fDegree;
}

void CTank::Set_PoSinpRotation(float fDegree)
{
	m_fCamPosinRot = fDegree;
}

void CTank::Set_Other_PotapRotation(float fDegree)
{
	m_fPotapRotation = fDegree;
}

void CTank::Set_Other_PoSinpRotation(float fDegree)
{
	m_fPosinRotation = fDegree;
}


void CTank::Set_Potap_For_Driver(float fDegree) {
	
	m_fPotapRotation = fDegree;
}

void CTank::Set_Posin_For_Driver(float fDegree)
{
	m_fPosinRotation = fDegree;
}

void CTank::Set_ShotDir(XMVECTOR Vec)
{
	vShotDir = Vec;
}

void CTank::Set_ShotMatrix(_matrix mat)
{
	ShotMatrix = mat;
}

void CTank::Respawn()
{
	if (_respawnTimer >= 5.f) {
		_respawnTimer = 0.f;

		_isSpawn = true;
	}

}

void CTank::Free()
{
	__super::Free();
	Safe_Release(m_VIBuffer);
	Safe_Release(m_CBBindingQuad);
	Safe_Release(m_VIBufferQuad);
	Safe_Release(m_QuadWorldTransform);
	Safe_Release(m_QuadTexTransform);
}

CTank* CTank::Create()
{
	CTank* pInstance = new CTank;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CRenderObject* CTank::Clone(void* pArg)
{
	CTank* pInstance = new CTank(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}

void CTank::OnSpawnAudio()
{
	if (!_myPlayer) {
		// 혹시 복제/상태 전환으로 남아있던 채널 정리
		if (_engineCh) { _engineCh->stop(); _engineCh = nullptr; }
		if (_trackCh) { _trackCh->stop();  _trackCh = nullptr; }
		return;
	}

	if (_engineCh && _trackCh) return; // 이미 생성됨

	// 현재 위치
	_vector posV = m_TransformCom->Get_State(CTransform::STATE_POSITION);
	XMFLOAT3 cur; XMStoreFloat3(&cur, posV);
	AudioVec3 p{ cur.x, cur.y, cur.z };
	AudioVec3 v{ 0,0,0 };

	// 엔진/궤도 루프 채널 생성(0 볼륨으로 시작 후 UpdateAudio에서 조절)
	if (!_engineCh)
		FMOD_Manager::Get_Instance()->Play3D_ReturnChannel("Tank_Engine_Sound", p, v, &_engineCh, 0.0f, false);
	if (!_trackCh)
		FMOD_Manager::Get_Instance()->Play3D_ReturnChannel("Tank_Track_Sound", p, v, &_trackCh, 0.0f, false);

}

void CTank::OnDespawnAudio()
{
	if (_engineCh) { _engineCh->stop(); _engineCh = nullptr; }
	if (_trackCh) { _trackCh->stop();  _trackCh = nullptr; }
}

void CTank::UpdateAudio(float dt)
{
	if (!_myPlayer || !_isSpawn) return;

	// 내 오디오 채널이 아직 없으면 생성(최초 1회)
	if (!_engineCh || !_trackCh) OnSpawnAudio();


	if (Network_Manager::GetInstance()->myPosition == POS_POSU && _useNetMix)
	{
		_netHoldT += dt;
		if (_netHoldT > 0.25f) {
			_useNetMix = false;
		}
		else {
			AudioVec3 p = ToAudio(m_TransformCom->Get_State(CTransform::STATE_POSITION));
			FMOD_VECTOR fp{ p.x,p.y,p.z }, fv{ 0,0,0 };
			if (_engineCh) { _engineCh->set3DAttributes(&fp, &fv); _engineCh->setVolume(_netEngVol); _engineCh->setPitch(_netEngPit); }
			if (_trackCh) { _trackCh->set3DAttributes(&fp, &fv); _trackCh->setVolume(_netTrkVol); _trackCh->setPitch(_netTrkPit); }
			return;
		}
	}
	else {

		bool anyDrive = (m_TankConsrolState.leftThrust || m_TankConsrolState.rightThrust ||
			m_TankConsrolState.leftReverse || m_TankConsrolState.rightReverse);
		SetIsMoving(anyDrive);
		SetRpmInput01(anyDrive ? 1.0f : 0.0f);

		// 2) RPM 스무딩
		float target = _rpmInput;
		const float up = _rpmRise * dt;
		const float down = _rpmFall * dt;

		if (_rpmSm < target) {
			_rpmSm = (std::min)(_rpmSm + up, target);     // 상승: target을 넘지 않게
		}
		else if (_rpmSm > target) {
			_rpmSm = (std::max)(_rpmSm - down, target);   // 감속: target 밑으로 떨어지지 않게
		}

		// 3) 현재 3D 위치/속도 적용
		AudioVec3 p = ToAudio(m_TransformCom->Get_State(CTransform::STATE_POSITION));
		AudioVec3 v = AudioVec3(0, 0, 0); // 물리 속도 벡터가 있으면 대입
		if (_engineCh) { FMOD_VECTOR fp{ p.x,p.y,p.z }, fv{ v.x,v.y,v.z }; _engineCh->set3DAttributes(&fp, &fv); }
		if (_trackCh) { FMOD_VECTOR fp{ p.x,p.y,p.z }, fv{ v.x,v.y,v.z }; _trackCh->set3DAttributes(&fp, &fv); }

		// 4) 엔진: 볼륨/피치는 RPM 기반
		//if (_engineCh) {
		//	float enginevol = _engineVolBase + _engineVolGain * _rpmSm;                         // 0.35 ~ 0.80
		//	float pitch = _enginePitchLo + (_enginePitchHi - _enginePitchLo) * _rpmSm;      // 0.95 ~ 1.30
		//	_engineCh->setVolume(enginevol);
		//	_engineCh->setPitch(pitch);
		//}

		//// 5) 궤도: 움직일 때만 들리게(볼륨/피치도 RPM 기반)
		//if (_trackCh) {
		//	float moveGate = _isMoving ? 1.0f : 0.0f;
		//	float drive = moveGate * (0.25f + 0.75f * _rpmSm); // 이동 중 최소 볼륨 보장
		//	float vol = (_trackVolBase + _trackVolGain * drive) * _trackMixGain;
		//	float pitch = _trackPitchLo + (_trackPitchHi - _trackPitchLo) * _rpmSm;


		//	_trackCh->setVolume(vol);
		//	_trackCh->setPitch(pitch);
		//}

		float engVol = _engineVolBase + _engineVolGain * _rpmSm;
		float engPit = _enginePitchLo + (_enginePitchHi - _enginePitchLo) * _rpmSm;

		float moveGate = _isMoving ? 1.0f : 0.0f;
		float drive = moveGate * (0.25f + 0.75f * _rpmSm);
		float trkVol = (_trackVolBase + _trackVolGain * drive) * _trackMixGain;
		float trkPit = _trackPitchLo + (_trackPitchHi - _trackPitchLo) * _rpmSm;

		if (_engineCh) { _engineCh->setVolume(engVol); _engineCh->setPitch(engPit); }
		if (_trackCh) { _trackCh->setVolume(trkVol); _trackCh->setPitch(trkPit); }

		if (!Network_Manager::GetInstance()->isConnected()) return;

		auto sendBuffer = ClientPacketHandler::Make_C_SOUND(engVol, engPit, trkVol, trkPit); // 숫자 그대로
		ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);

	}

}

void CTank::SetSoundData(float engVol, float engPit, float trkVol, float trkPit)
{

	_netEngVol = engVol;
	_netEngPit = engPit;
	_netTrkVol = trkVol;
	_netTrkPit = trkPit;
	_useNetMix = true;
	_netHoldT = 0.f;
}


void CTank::RotPotap_And_Posin(float fTimeDelta)
{

	if (m_fPotapRotation <= m_fCamPotapRot - 0.5f)
		m_fPotapRotation += 20.f * fTimeDelta;
	else if (m_fPotapRotation >= m_fCamPotapRot + 0.5f)
		m_fPotapRotation -= 20.f * fTimeDelta;
	else
		m_fPotapRotation = m_fCamPotapRot;

	if (m_fPosinRotation <= m_fCamPosinRot - 0.5f)
		m_fPosinRotation += 20.f * fTimeDelta;
	else if (m_fPosinRotation >= m_fCamPosinRot + 0.5f)
		m_fPosinRotation -= 20.f * fTimeDelta;
	else
		m_fPosinRotation = m_fCamPosinRot;
}

void CTank::Request_Air_Drop()
{

	if (!_airdropMode) return;     // 모드일 때만 입력 처리
	if (!_airdropReady) return;    // 쿨타임 중 차단
	if (!Network_Manager::GetInstance()->isConnected()) return;

	for (uint8 slot = 1; slot <= 9; ++slot)
	{
		char key = '0' + slot; // '1'~'9'
		if (m_GameInstance->Key_Down(key))
		{
			auto sendBuffer = ClientPacketHandler::Make_C_AIRDROP(slot); // 숫자 그대로
			ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);

			// 쿨타임 시작
			_airdropReady = false;
			_airdropTimer = 0.f;

			// TODOUI: 성공 효과, 알림 등
			Exit_AirDropMode(); // 1회 선택 후 모드 종료
			break;
		}
	}

	// 취소 키(ESC)로 모드 종료 옵션
	if (m_GameInstance->Key_Down(27)) { // 27 == ESC
		Exit_AirDropMode();
	}
}

void CTank::Enter_Air_DropMode()
{
	if (CanEnterAirDropMode()) {
		Start_AirDropMode();
		// UI가 떠 있는 동안 숫자키 입력을 받음
		Request_Air_Drop();
	}
	else {
		// TODOUI: 쿨타임 남았거나 조건 미충족 메시지
		// 예) 남은 쿨타임 표시도 가능: AIRDROP_COOLDOWN_SEC - _airdropTimer
	}

}

void CTank::UpdateAirDropCooldown(float dt)
{
	if (_airdropReady) return;
	_airdropTimer += dt;
	if (_airdropTimer >= AIRDROP_COOLDOWN_SEC) {
		_airdropTimer = 0.f;
		_airdropReady = true;
		// TODOUI: UI에 "에어드랍 준비 완료" 같은 표시 가능
	}
}

bool CTank::CanEnterAirDropMode() const
{
	if (!_isSpawn) return false;
	if (!Network_Manager::GetInstance()->isConnected()) return true;
	if (!_airdropReady) return false;
	//if (Network_Manager::GetInstance()->MyControlTarget != CONTROL_POSIN) return false;
	return true;
}

void CTank::Start_AirDropMode()
{
	_airdropMode = true;
	((CUI_AirDrop*)(CGameInstance::Get_Instance()->GetGameObject("UI", UI_AIRDROP)))->set_render();
	// TODOUI: 에어드랍 영역 선택 UI 켜기
}

void CTank::Exit_AirDropMode()
{
	_airdropMode = false;
	((CUI_AirDrop*)(CGameInstance::Get_Instance()->GetGameObject("UI", UI_AIRDROP)))->set_render_off();
	// TODOUI: 에어드랍 선택 UI 끄기
}

void CTank::Master_Pos_KeyInput()
{
	if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_TANK) {
		if (m_GameInstance->Key_Down('M')) {
			_respawnTimer = 0.f;
			_isSpawn = false;
		}

		if (m_GameInstance->Key_Down('W'))
			m_TankConsrolState.leftThrust = true;

		if (m_GameInstance->Key_Down('S'))
			m_TankConsrolState.leftBrake = true;

		if (m_GameInstance->Key_Down('R'))
			m_TankConsrolState.rightThrust = true;

		if (m_GameInstance->Key_Down('F'))
			m_TankConsrolState.rightBrake = true;

		if (m_GameInstance->Key_Down('X'))
			m_TankConsrolState.leftReverse = true;

		if (m_GameInstance->Key_Down('V'))
			m_TankConsrolState.rightReverse = true;

		if (m_GameInstance->Key_Up('W'))
			m_TankConsrolState.leftThrust = false;

		if (m_GameInstance->Key_Up('S'))
			m_TankConsrolState.leftBrake = false;

		if (m_GameInstance->Key_Up('R'))
			m_TankConsrolState.rightThrust = false;

		if (m_GameInstance->Key_Up('F'))
			m_TankConsrolState.rightBrake = false;

		if (m_GameInstance->Key_Up('X'))
			m_TankConsrolState.leftReverse = false;

		if (m_GameInstance->Key_Up('V'))
			m_TankConsrolState.rightReverse = false;


	}
	else if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_POSIN) {

		if (m_GameInstance->Mouse_Down(0)) {

			if (_shootTimer >= SHOOT_INTERVAL)
			{
				if (Network_Manager::GetInstance()->isConnected()) {
					SendShootDataToServer(); // 실제 슈팅
					((CUIReloading*)m_GameInstance->GetGameObject("UI", UI_RELOADING))->Set_Reloading();
					dynamic_cast<CCamera_Free*>(m_GameInstance->GetGameObject("Camera", 0))->StartShake(0.7f, 0.7f, 40.f);

				}
				_shootTimer = 0.f; // 타이머 초기화
			}


		}
		if (m_GameInstance->Key_Down('T')) {
			Enter_Air_DropMode();
		}
		// 모드 활성 중이면 숫자 입력 처리
		if (_airdropMode) {
			Request_Air_Drop(); // 한 번 선택되면 Exit_AirDropMode()에서 빠져나감
		}
	}
	
	m_pPhysicsEngine->Set_Tank_ControlState(m_TankConsrolState);

	const bool anyDrive =
		m_TankConsrolState.leftThrust || m_TankConsrolState.rightThrust ||
		m_TankConsrolState.leftReverse || m_TankConsrolState.rightReverse;

	SetIsMoving(anyDrive);                 // 궤도 소리 on/off 기준
	SetRpmInput01(anyDrive ? 1.0f : 0.0f);// 엔진/궤도 RPM 타깃(0~1, UpdateAudio에서 스무딩)

}

void CTank::Driver_Pos_KeyInput()
{

	if (m_GameInstance->Key_Down('W'))
		m_TankConsrolState.leftThrust = true;

	if (m_GameInstance->Key_Down('S'))
		m_TankConsrolState.leftBrake = true;

	if (m_GameInstance->Key_Down('R'))
		m_TankConsrolState.rightThrust = true;

	if (m_GameInstance->Key_Down('F'))
		m_TankConsrolState.rightBrake = true;

	if (m_GameInstance->Key_Down('X'))
		m_TankConsrolState.leftReverse = true;

	if (m_GameInstance->Key_Down('V'))
		m_TankConsrolState.rightReverse = true;

	if (m_GameInstance->Key_Up('W'))
		m_TankConsrolState.leftThrust = false;

	if (m_GameInstance->Key_Up('S'))
		m_TankConsrolState.leftBrake = false;

	if (m_GameInstance->Key_Up('R'))
		m_TankConsrolState.rightThrust = false;

	if (m_GameInstance->Key_Up('F'))
		m_TankConsrolState.rightBrake = false;

	if (m_GameInstance->Key_Up('X'))
		m_TankConsrolState.leftReverse = false;

	if (m_GameInstance->Key_Up('V'))
		m_TankConsrolState.rightReverse = false;


	m_pPhysicsEngine->Set_Tank_ControlState(m_TankConsrolState);


	const bool anyDrive =
		m_TankConsrolState.leftThrust || m_TankConsrolState.rightThrust ||
		m_TankConsrolState.leftReverse || m_TankConsrolState.rightReverse;

	SetIsMoving(anyDrive);                 // 궤도 소리 on/off 기준
	SetRpmInput01(anyDrive ? 1.0f : 0.0f);// 엔진/궤도 RPM 타깃(0~1, UpdateAudio에서 스무딩)
}

void CTank::POSU_Pos_KeyInput()
{
	if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_POSIN) {

		if (m_GameInstance->Mouse_Down(0)) {
			if (_shootTimer >= SHOOT_INTERVAL)
			{
				if (Network_Manager::GetInstance()->isConnected()) {
					SendShootDataToServer();
					((CUIReloading*)m_GameInstance->GetGameObject("UI", UI_RELOADING))->Set_Reloading();
					dynamic_cast<CCamera_Free*>(m_GameInstance->GetGameObject("Camera", 0))->StartShake(0.7f, 0.7f, 40.f);
				}

				_shootTimer = 0.f;
			}

		}

		if (m_GameInstance->Key_Down('T')) {
			Enter_Air_DropMode();
		}
		// 모드 활성 중이면 숫자 입력 처리
		if (_airdropMode) {
			Request_Air_Drop(); // 한 번 선택되면 Exit_AirDropMode()에서 빠져나감
		}
	}
}

void CTank::Set_Tank_Element_from_Engine()
{
	PxMat44 pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_CHASSIS);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
	);

	_float4x4 matworld;
	XMStoreFloat4x4(&matworld, mat);

	m_TransformCom->Set_WorldMatrix(matworld);

#pragma region Wheel

	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_FIRST_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX L1Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_SECOND_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX R1Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_THIRD_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX L2Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_FOURTH_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX R2Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_FIFTH_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX L3Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_SIXTH_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX R3Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_SEVENTH_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX L4Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_FIRST_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX R4Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_SECOND_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX L5Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_THIRD_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX R5Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_FOURTH_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX L6Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_FIFTH_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX R6Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_SIXTH_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX L7Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);

	pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_SEVENTH_WHEEL);

	// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
	XMMATRIX R7Mat = XMMATRIX(
		XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
		XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
		XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
		XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
	);
#pragma endregion


	_vector forward = m_TransformCom->Get_State(CTransform::STATE_LOOK);
	float yaw = atan2(XMVectorGetX(forward), XMVectorGetZ(forward));
	//float yawDeg = XMConvertToDegrees(yaw);

	XMMATRIX matPotap = XMMatrixRotationY(XMConvertToRadians(m_fPotapRotation - XMConvertToDegrees(yaw)));

	XMMATRIX matPosin = XMMatrixRotationY(XMConvertToRadians(m_fPosinRotation));

	//if(m_fPosinRotation  )

	/*XMMATRIX matPosinforDir = XMMatrixRotationX(-m_fPosinRotation);

	_vector worldforward = XMVectorSet(0, 0, 1, 0);
	worldforward = XMVector4Transform(worldforward, m_TransformCom->Get_WorldMatrix());
	worldforward = XMVector4Transform(worldforward, matPotap);
	worldforward = XMVector4Transform(worldforward, matPosinforDir);*/

	/*XMFLOAT3 vRight;
	XMFLOAT3 vUp;*/

	m_VIBuffer->Set_Transform_Matrix(0, mat); // Chassis
	m_VIBuffer->Set_Transform_Matrix(1, matPotap); // Potap
	//m_VIBuffer->Set_Transform_Matrix(2, matPosin);


	m_VIBuffer->Set_Transform_Matrix(24 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L1Mat); // Left First Wheel
	m_VIBuffer->Set_Transform_Matrix(26 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L2Mat); // Left Second Wheel
	m_VIBuffer->Set_Transform_Matrix(28 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L3Mat); // Left Third Wheel
	m_VIBuffer->Set_Transform_Matrix(30 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L4Mat); // Left Fourth Wheel
	m_VIBuffer->Set_Transform_Matrix(32 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L5Mat); // Left Fifth Wheel
	m_VIBuffer->Set_Transform_Matrix(34 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L6Mat); // Left Sixth Wheel
	m_VIBuffer->Set_Transform_Matrix(36 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L7Mat); // Left Seventh Wheel
	m_VIBuffer->Set_Transform_Matrix(46 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R1Mat); // Right First Wheel
	m_VIBuffer->Set_Transform_Matrix(37 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R2Mat); // Right Second Wheel
	m_VIBuffer->Set_Transform_Matrix(35 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R3Mat); // Right Third Wheel
	m_VIBuffer->Set_Transform_Matrix(33 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R4Mat); // Right Fourth Wheel
	m_VIBuffer->Set_Transform_Matrix(44 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R5Mat); // Right Fifth Wheel
	m_VIBuffer->Set_Transform_Matrix(48 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R6Mat); // Right Sixth Wheel
	m_VIBuffer->Set_Transform_Matrix(42 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R7Mat); // Right Seventh Wheel


	//   m_VIBuffer->Set_Transform_Matrix(0, m_TransformCom->Get_WorldMatrix()); // Chassis
	   //m_VIBuffer->Set_Transform_Matrix(1, m_TransformCom->Get_WorldMatrix()); // Potap

	m_VIBuffer->Invalidate_Bones();

	m_VIBuffer->Multiply_Mesh_Combined_Matrix(50, matPosin);
	m_VIBuffer->Multiply_Mesh_Combined_Matrix(51, matPosin);
	m_VIBuffer->Multiply_Mesh_Combined_Matrix(29, matPosin);

	m_VIBuffer->Update();
}

void CTank::Set_Tank_Element_from_ServerData()
{
	/*XMMATRIX matPotap = XMMatrixRotationY(m_fPotapRotation);

			XMMATRIX matPosin = XMMatrixRotationY(m_fPosinRotation);*/

	_vector forward = m_TransformCom->Get_State(CTransform::STATE_LOOK);
	float yaw = atan2(XMVectorGetX(forward), XMVectorGetZ(forward));
	//float yawDeg = XMConvertToDegrees(yaw);

	XMMATRIX matPotap = XMMatrixRotationY(XMConvertToRadians(m_fPotapRotation - XMConvertToDegrees(yaw)));

	XMMATRIX matPosin = XMMatrixRotationY(XMConvertToRadians(m_fPosinRotation));


	m_VIBuffer->Set_Transform_Matrix(0, m_TransformCom->Get_WorldMatrix()); // Chassis
	m_VIBuffer->Set_Transform_Matrix(1, matPotap); // Potap
	//m_VIBuffer->Set_Transform_Matrix(2, matPosin); // Posin

	//여기서 받은 데이터로 매트릭스 바꿔줌
	m_VIBuffer->Invalidate_Bones();

	m_VIBuffer->Set_Combined_Matrix(26, m_VIBuffer->Get_TransformMatrix(26) * m_TransformCom->Get_WorldMatrix()); // Left Second Wheel
	m_VIBuffer->Set_Combined_Matrix(28, m_VIBuffer->Get_TransformMatrix(28) * m_TransformCom->Get_WorldMatrix()); // Left Third Wheel
	m_VIBuffer->Set_Combined_Matrix(24, m_VIBuffer->Get_TransformMatrix(24) * m_TransformCom->Get_WorldMatrix()); // Left First Wheel
	m_VIBuffer->Set_Combined_Matrix(30, m_VIBuffer->Get_TransformMatrix(30) * m_TransformCom->Get_WorldMatrix()); // Left Fourth Wheel
	m_VIBuffer->Set_Combined_Matrix(32, m_VIBuffer->Get_TransformMatrix(32) * m_TransformCom->Get_WorldMatrix()); // Left Fifth Wheel
	m_VIBuffer->Set_Combined_Matrix(34, m_VIBuffer->Get_TransformMatrix(34) * m_TransformCom->Get_WorldMatrix()); // Left Sixth Wheel
	m_VIBuffer->Set_Combined_Matrix(36, m_VIBuffer->Get_TransformMatrix(36) * m_TransformCom->Get_WorldMatrix()); // Left Seventh Wheel
	m_VIBuffer->Set_Combined_Matrix(46, m_VIBuffer->Get_TransformMatrix(46) * m_TransformCom->Get_WorldMatrix()); // Right First Wheel
	m_VIBuffer->Set_Combined_Matrix(37, m_VIBuffer->Get_TransformMatrix(37) * m_TransformCom->Get_WorldMatrix()); // Right Second Wheel
	m_VIBuffer->Set_Combined_Matrix(35, m_VIBuffer->Get_TransformMatrix(35) * m_TransformCom->Get_WorldMatrix()); // Right Third Wheel
	m_VIBuffer->Set_Combined_Matrix(33, m_VIBuffer->Get_TransformMatrix(33) * m_TransformCom->Get_WorldMatrix()); // Right Fourth Wheel
	m_VIBuffer->Set_Combined_Matrix(44, m_VIBuffer->Get_TransformMatrix(44) * m_TransformCom->Get_WorldMatrix()); // Right Fifth Wheel
	m_VIBuffer->Set_Combined_Matrix(48, m_VIBuffer->Get_TransformMatrix(48) * m_TransformCom->Get_WorldMatrix()); // Right Sixth Wheel
	m_VIBuffer->Set_Combined_Matrix(42, m_VIBuffer->Get_TransformMatrix(42) * m_TransformCom->Get_WorldMatrix()); // Right Seventh Wheel

	//i == 24 || i == 26 || i == 28 || i == 30 || i == 32 || i == 34 || i == 36 || i == 46 || i == 37 || i == 35 || i == 33 || i == 44 || i == 48 || i == 42

	m_VIBuffer->Multiply_Mesh_Combined_Matrix(50, matPosin);
	m_VIBuffer->Multiply_Mesh_Combined_Matrix(51, matPosin);
	m_VIBuffer->Multiply_Mesh_Combined_Matrix(29, matPosin);

	((CModel*)m_VIBuffer)->Update();
}

void CTank::ErrorRespawn()
{
	_vector vCurPos = m_TransformCom->Get_State(CTransform::STATE_POSITION);
	float y = XMVectorGetY(vCurPos);

	if (y < -60.f)
	{
		float safeX = XMVectorGetX(vCurPos);
		float safeZ = XMVectorGetZ(vCurPos);
		float safeY = 40.0f; // 안전한 높이

		// 물리 위치 재설정
		m_pPhysicsEngine->Set_Pos(safeX, safeY, safeZ);

		// Transform 동기화 (옵션, 프레임에 따라 자동 동기화 될 수도 있음)
		_vector safePos = XMVectorSet(safeX, safeY, safeZ, 1.0f);
		m_TransformCom->Set_State(CTransform::STATE_POSITION, safePos);

	}
}



void CTank::SendMyStateToServer()
{
	_float4x4 TempMat;
	XMStoreFloat4x4(&TempMat, m_TransformCom->Get_WorldMatrix());
	auto sendBuffer = ClientPacketHandler::Make_C_MOVE(TempMat, m_fPotapRotation, m_fPosinRotation);
	ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);
}

void CTank::SendPosinData() {

	_float4x4 TempMat;
	XMStoreFloat4x4(&TempMat, m_TransformCom->Get_WorldMatrix());
	auto sendBuffer = ClientPacketHandler::Make_C_TANK_POSINMOVE(m_fPotapRotation, m_fPosinRotation);
	ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);
}

void CTank::SendPosData() {

	_float4x4 TempMat;
	XMStoreFloat4x4(&TempMat, m_TransformCom->Get_WorldMatrix());
	auto sendBuffer = ClientPacketHandler::Make_C_TANK_POSMOVE(TempMat);
	ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);
}

void CTank::SendShootDataToServer()
{
	_vector forward = m_TransformCom->Get_State(CTransform::STATE_LOOK);
	float yaw = atan2(XMVectorGetX(forward), XMVectorGetZ(forward));

	// 1. 기준이 될 탱크의 순수 전방 벡터를 가져옵니다.
	_vector vBaseLook = XMVector3Normalize(m_TransformCom->Get_State(CTransform::STATE_LOOK));

	// 2. 포탑의 좌우 회전(Yaw) 행렬을 만듭니다.
	_matrix matYaw = XMMatrixRotationAxis(m_TransformCom->Get_State(CTransform::STATE_UP),
		XMConvertToRadians(m_fPotapRotation - XMConvertToDegrees(yaw)));

	// 3. 포신의 상하 회전(Pitch) 행렬을 만듭니다.
	_matrix matPitch = XMMatrixRotationAxis(m_TransformCom->Get_State(CTransform::STATE_RIGHT),
		XMConvertToRadians(m_fPosinRotation));

	// 4. 최종 회전 행렬 계산 (중요: Pitch를 먼저 곱하고 Yaw를 곱합니다)
	// 로컬 축 기준의 Pitch 변환이 먼저 적용되고, 그 결과가 Yaw 변환됩니다.
	_matrix matTotalRotation = matPitch * matYaw;

	// 5. 최종 방향 벡터를 계산합니다.
	// 방향 벡터에는 이동(Translation) 정보가 포함되면 안 되므로 XMVector3TransformNormal을 사용하는 것이 더 안전합니다.
	_vector vDir = XMVector3TransformNormal(vBaseLook, matTotalRotation);

	_vector vPos = m_TransformCom->Get_State(CTransform::STATE_POSITION) + m_TransformCom->Get_State(CTransform::STATE_UP) * 1.95f
		+ XMVector3TransformNormal(vBaseLook, matYaw) * 1.15f + vDir * 3.8f;

	// 최종 결과를 _float3로 저장 (기존과 동일)
	_float3 fPos, fDir;
	XMStoreFloat3(&fPos, vPos);
	XMStoreFloat3(&fDir, vDir);


	float data[6] = {
	   fPos.x, fPos.y, fPos.z,  // 위치
	   fDir.x, fDir.y, fDir.z   // 정규화된 방향
	};

	auto sendBuffer = ClientPacketHandler::Make_C_SHOT(fPos.x, fPos.y, fPos.z,
		fDir.x, fDir.y, fDir.z);
	ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);
}

void CTank::PushBulletMatrix(const _matrix& mat)
{
	std::lock_guard<std::mutex> lock(BulletQueueMutex);
	BulletQueue.push(mat);
}

void CTank::PopAllBulletMatrix(std::function<void(const _matrix&)> processFunc)
{
	std::lock_guard<std::mutex> lock(BulletQueueMutex);
	while (!BulletQueue.empty())
	{
		const _matrix& mat = BulletQueue.front();
		processFunc(mat);
		BulletQueue.pop();
	}
}

void CTank::Set_OtherPlayerState(_float4x4 mat, float PotapRot, float PosinRot)
{

	m_TransformCom->Set_WorldMatrix(mat);
	Set_Other_PotapRotation(PotapRot);
	Set_Other_PoSinpRotation(PosinRot);

}


void CTank::Set_Posin(float PotapRot, float PosinRot)
{

	Set_PotapRotation(PotapRot);
	Set_PoSinpRotation(PosinRot);

}

void CTank::Set_DriverModeData(float PotapRot, float PosinRot) {
	Set_Potap_For_Driver(PotapRot);
	Set_Posin_For_Driver(PosinRot);

}


void CTank::Set_MyPos(float x, float y, float z)
{
	m_pPhysicsEngine->Set_Pos(x, y, z);

	_vector safePos = XMVectorSet(x, y, z, 1.0f);
	m_TransformCom->Set_State(CTransform::STATE_POSITION, safePos);


}


void CTank::Tick_For_Posin_Image(float fTimeDelta)
{
	//m_RendererCom->AddtoRenderObjects(m_RGQuad, this);

	m_QuadWorldTransform->Identity();

	m_QuadWorldTransform->Set_Scale(CTransform::STATE_UP, 1.7777f);

	// 여기까지는 기본으로 해줘야 하는 것.

	float angleDiffX = m_fPotapRotation - m_fCamPotapRot;
	float angleDiffY = m_fCamPosinRot - m_fPosinRotation;

	//// 스케일링.
	//if (angleDiffX == 0.f && angleDiffY == 0.f) {
	//	m_fSameTime += fTimeDelta;
	//	if (m_fSameTime > 3.f)
	//		m_fSameTime = 3.f;
	//}
	//else
	//{
	//	/*m_fSameTime -= fTimeDelta;
	//	if (m_fSameTime < 0)
	//		m_fSameTime = 0.f;*/
	//	m_fSameTime = 0.f;
	//}

	//float fScale = 1.f - m_fSameTime * 0.29f;

	m_QuadWorldTransform->Set_Scale(0.05f);

	// 위치 바꾸기
	angleDiffX /= 30.f;
	angleDiffY /= 30.f;

	m_QuadWorldTransform->Set_State(CTransform::STATE_POSITION, XMVectorSet(angleDiffX, angleDiffY, 0, 1));

	m_CBBindingQuad->Set_World_TexCoord_And_Update(m_QuadWorldTransform, m_QuadTexTransform);
}

void CTank::Render_For_Posin_Image()
{
	m_CBBindingQuad->Set_On_Shader();

	m_VIBufferQuad->Render();
}

void CTank::CheckRespawnKeyInput()
{
	if (m_GameInstance->Key_Down('1')) Choiced_Pos = 1;
	else if (m_GameInstance->Key_Down('2')) Choiced_Pos = 2;
	else if (m_GameInstance->Key_Down('3')) Choiced_Pos = 3;
	else if (m_GameInstance->Key_Down('4')) Choiced_Pos = 4;
	else if (m_GameInstance->Key_Down('5')) Choiced_Pos = 5;
	else if (m_GameInstance->Key_Down('6')) Choiced_Pos = 6;
	else if (m_GameInstance->Key_Down('7')) Choiced_Pos = 7;
	else if (m_GameInstance->Key_Down('8')) Choiced_Pos = 8;

}

void CTank::setRespawn() {
	if (!is_RespawnArea_choiced) {
		CheckRespawnKeyInput();

		if (Choiced_Pos != 0)
		{
			is_RespawnArea_choiced = true;
		}
	}
	else {
		Network_Manager::GetInstance()->ReSpawn();
		((CUISelectPos*)m_GameInstance->GetGameObject("UI", UI_SELECT_POS))->set_render_off();
		_isSpawn = true;
		_respawnTimer = 0.f;
		is_RespawnArea_choiced = false;
		XMFLOAT3 respawnPosVec;

		switch (Choiced_Pos)
		{
		case 1: respawnPosVec = XMFLOAT3(RESPAWNPOS_1); break;
		case 2: respawnPosVec = XMFLOAT3(RESPAWNPOS_2); break;
		case 3: respawnPosVec = XMFLOAT3(RESPAWNPOS_3); break;
		case 4: respawnPosVec = XMFLOAT3(RESPAWNPOS_4); break;
		case 5: respawnPosVec = XMFLOAT3(RESPAWNPOS_5); break;
		case 6: respawnPosVec = XMFLOAT3(RESPAWNPOS_6); break;
		case 7: respawnPosVec = XMFLOAT3(RESPAWNPOS_7); break;
		case 8: respawnPosVec = XMFLOAT3(RESPAWNPOS_8); break;
		default: respawnPosVec = XMFLOAT3(0.f, 40.f, 0.f); break;
		}

		// 물리엔진 위치 설정
		m_pPhysicsEngine->Set_Pos(respawnPosVec.x, respawnPosVec.y, respawnPosVec.z);

		// Transform도 이동
		_vector respawnPos = XMVectorSet(respawnPosVec.x, respawnPosVec.y, respawnPosVec.z, 1.f);
		m_TransformCom->Set_State(CTransform::STATE_POSITION, respawnPos);

		_float4x4 TempMat;
		XMStoreFloat4x4(&TempMat, m_TransformCom->Get_WorldMatrix());
		if (Network_Manager::GetInstance()->isConnected()) {
			auto sendBuffer = ClientPacketHandler::Make_C_TANK_RESPAWN(TempMat, m_fPotapRotation, m_fPosinRotation);
			ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);
		}

		dynamic_cast<CDrone*>(CGameInstance::Get_Instance()->GetGameObject("Drone", Network_Manager::GetInstance()->GetMyTankIndex()))->Set_My_DronePos_OnTank(TempMat);
		Choiced_Pos = 0;
	}
}

void CTank::setRespawnForPosinMode()
{
	_isSpawn = true;
	Network_Manager::GetInstance()->ReSpawn();
}

