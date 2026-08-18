#include "Client_pch.h"
#include "Tank.h"
#include "Client_Defines.h"
#include "GameInstance.h"
#include "ClientPacketHandler.h"
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

/*  이 클라가 서버로 상태를 올리는 주기. 렌더 프레임과 무관하다.
	서버 틱이 60Hz 라 그보다 자주 보내봐야 다음 틱 전에 덮여서 그냥 버려진다. */
static const float NET_SEND_INTERVAL = 1.f / 30.f;



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

	/* 바퀴 본 행렬이 매 프레임 PhysX 값으로 덮이기 전인 '지금' 쉴 때 위치를 받아둬야 한다.
	   LateTick 에서 부르면 이미 덮인 뒤라 값이 전부 0 이 된다. */
	Ready_Track_Sag();

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

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Posin", CTexture::Create(L"../bin/Models/FinalUI/cross.dds"));

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

	/*  ★ 반드시 Set_Tank_Element_from_ServerData() 보다 먼저 부를 것.
		그 함수가 m_TransformCom / m_fPotapRotation 을 읽어 뼈에 꽂으므로,
		여기서 이번 프레임 값을 만들어 두지 않으면 보간이 한 프레임 늦게 반영된다.  */
	Update_NetInterpolation(fTimeDelta);

	if (_myPlayer) {

		/*  송신 주기를 렌더 프레임에서 떼어낸다. */
		m_fNetSendTimer += fTimeDelta;

		const bool isSendTurn = (m_fNetSendTimer >= NET_SEND_INTERVAL);
		if (isSendTurn)
		{
			m_fNetSendTimer -= NET_SEND_INTERVAL;

			/* 프레임이 크게 밀렸다면 밀린 만큼 몰아 보내지 않는다(따라잡아도 의미 없는 과거 상태다) */
			if (m_fNetSendTimer > NET_SEND_INTERVAL)
				m_fNetSendTimer = 0.f;
		}

		const bool isSendable = Network_Manager::GetInstance()->isConnected() && isSendTurn;

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
			if (isSendable)
				SendMyStateToServer();

		}
					   break;
		case POS_DRIVER: {
			Set_Tank_Element_from_Engine();
			ErrorRespawn();
			if (isSendable)
				SendPosData();

		}
					   break;
		case POS_POSU: {

			Set_Tank_Element_from_ServerData();
			Tick_For_Posin_Image(fTimeDelta);
			RotPotap_And_Posin(fTimeDelta);
			if (isSendable)
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

				//m_RendererCom->ChangePSO("UIPSO");
				if (m_isQuadTurn < 2)
				{
					m_VIBuffer->Render(50);
					m_isQuadTurn += 1;
				}
				
				//m_RendererCom->ChangePSO("DefaultPSO");
				else
				{
					Render_For_Posin_Image();
					m_isQuadTurn = 0;
				}
			}
			else
			{
				for (int i = 0; i < 55; ++i)
				{
					if (!((i >= 6 && i <= 12) || (i >= 15 && i <= 21)))
					{
						m_VIBuffer->Render(i);
					}
				}
			}


		}
			break;
		case POS_DRIVER:
		{
			for (int i = 0; i < 55; ++i)
			{
				if (!((i >= 6 && i <= 12) || (i >= 15 && i <= 21)))
				{
					m_VIBuffer->Render(i);
				}
			}
		}
			break;
		case POS_POSU:
		{
			if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_POSIN) {
				Render_For_Posin_Image();
			}
			else {
				for (int i = 0; i < 55; ++i)
				{
					if (!((i >= 6 && i <= 12) || (i >= 15 && i <= 21)))
					{
						m_VIBuffer->Render(i);
					}
				
				}
			}
		}
			break;
		default:
			break;
		}

	}
	else {

		for (int i = 0; i < 55; ++i)
		{
			if (!((i >= 6 && i <= 12) || (i >= 15 && i <= 21)))
			{
				m_VIBuffer->Render(i);
			}
		}
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

	


	if (Network_Manager::GetInstance()->MyPosMode == POS_POSU && _useNetMix)
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
		Network_Manager::GetInstance()->Send(sendBuffer);

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
			Network_Manager::GetInstance()->Send(sendBuffer);

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

	m_VIBuffer->Set_Transform_Matrix(0, mat); // Chassis
	m_VIBuffer->Set_Transform_Matrix(1, matPotap); // Potap
	//m_VIBuffer->Set_Transform_Matrix(2, matPosin);


	/* 바퀴 14개 + 궤도 2개를 여기서 한 번에 놓는다.. */
	const _matrix PhysXWheelMatrices[14] =
	{
		L1Mat, L2Mat, L3Mat, L4Mat, L5Mat, L6Mat, L7Mat,
		R1Mat, R2Mat, R3Mat, R4Mat, R5Mat, R6Mat, R7Mat,
	};
	Update_Wheels_And_Track(mat, PhysXWheelMatrices);



	//   m_VIBuffer->Set_Transform_Matrix(0, m_TransformCom->Get_WorldMatrix()); // Chassis
	   //m_VIBuffer->Set_Transform_Matrix(1, m_TransformCom->Get_WorldMatrix()); // Potap

	m_VIBuffer->Invalidate_Bones();

	m_VIBuffer->Multiply_Mesh_Combined_Matrix(50, matPosin);
	m_VIBuffer->Multiply_Mesh_Combined_Matrix(51, matPosin);
	m_VIBuffer->Multiply_Mesh_Combined_Matrix(29, matPosin);

	m_VIBuffer->Update();
}

/*===========================================================================
	궤도 구부리기

	메쉬 번호는 Model.cpp 의 s_ParentOfMesh 표와 같은 순서.
	왼쪽 바퀴는 앞에서 뒤로 24, 26, 28, 30, 32, 34, 36 이고
	오른쪽은 46, 37, 35, 33, 44, 48, 42.
	궤도는 38
===========================================================================*/
namespace
{
	const _uint TRACK_MESH[2] = { 38, 39 };
	const _uint WHEEL_MESH[2][7] =
	{
		{ 24, 26, 28, 30, 32, 34, 36 },		/* 왼쪽  : 앞 -> 뒤 */
		{ 46, 37, 35, 33, 44, 48, 42 },		/* 오른쪽: 앞 -> 뒤 */
	};

	/* 궤도 메쉬 로컬공간에서 윗런의 높이(z). +z 가 아래쪽이라 윗런이 제일 작은 값이다.
	   M1A2 궤도 메쉬를 실측한 값이고, 모델을 바꾸면 다시 재야 한다.
	   (바닥런은 0.81 인데, 바퀴 축 높이는 바퀴 본에서 직접 구하므로 상수가 필요 없다) */
	const float TRACK_TOP_RUN_Z = -0.38f;
}

void CTank::Ready_Track_Sag()
{
	/* 바퀴 본 행렬은 매 프레임 PhysX 값으로 덮이므로, 덮이기 전인 지금 쉴 때 위치를 받아둔다. */
	for (int iSide = 0; iSide < 2; ++iSide)
	{
		const _matrix TrackBone = m_VIBuffer->Get_TransformMatrix(TRACK_MESH[iSide]);
		const _matrix TrackBoneInverse = XMMatrixInverse(nullptr, TrackBone);

		float fFrontX = 0.f, fRearX = 0.f, fWheelLineZ = 0.f;

		for (_uint i = 0; i < TRACK_WHEEL_COUNT; ++i)
		{
			const _matrix WheelBone = m_VIBuffer->Get_TransformMatrix(WHEEL_MESH[iSide][i]);
			const _vector vWheelModelPos = WheelBone.r[3];

			XMStoreFloat4x4(&m_WheelRestMatrix[iSide][i], WheelBone);

			/* 같은 바퀴가 궤도 메쉬 안에서는 어디쯤인지(로컬 x, 로컬 z) */
			const _vector vWheelTrackPos = XMVector3TransformCoord(vWheelModelPos, TrackBoneInverse);

			if (i == 0)
				fFrontX = XMVectorGetX(vWheelTrackPos);
			if (i == TRACK_WHEEL_COUNT - 1)
				fRearX = XMVectorGetX(vWheelTrackPos);

			fWheelLineZ += XMVectorGetZ(vWheelTrackPos) / TRACK_WHEEL_COUNT;
		}

		/* 바퀴 축 높이(fWheelLineZ)부터 아래로는 궤도가 바퀴를 100% 따라가야 바퀴를 품는다.
		   그 위(윗런 쪽)로만 서서히 0 으로 뺀다. */
		m_TrackParam[iSide] = _float4(fFrontX, fRearX, fWheelLineZ, TRACK_TOP_RUN_Z);
	}

	m_isTrackSagReady = true;
}

void CTank::Update_Wheels_And_Track(_fmatrix ChassisWorld, const _matrix* pPhysXWheelMatrices)
{
	if (!m_isTrackSagReady)
		return;

	const _matrix ChassisInverse = XMMatrixInverse(nullptr, ChassisWorld);

	for (int iSide = 0; iSide < 2; ++iSide)
	{
		float fSag[TRACK_WHEEL_COUNT] = {};

		for (_uint i = 0; i < TRACK_WHEEL_COUNT; ++i)
		{
			const _matrix RestMatrix = XMLoadFloat4x4(&m_WheelRestMatrix[iSide][i]);

			/* PhysX 바퀴를 차체 기준으로 되돌려서, 쉴 때보다 얼마나 오르내렸는지만 뽑는다. */
			const _vector vPhysXModelPos = XMVector3TransformCoord(
				pPhysXWheelMatrices[iSide * TRACK_WHEEL_COUNT + i].r[3], ChassisInverse);

			fSag[i] = XMVectorGetY(vPhysXModelPos) - XMVectorGetY(RestMatrix.r[3]);

			/* 송신용으로 같이 담아둔다(int8 한 칸 = 0.0031). */
			m_WheelSagWire[iSide * TRACK_WHEEL_COUNT + i] = QuantizeWheelSag(fSag[i]);

			/* 바퀴 = 모델 제자리에서 위아래로만 이동 */
			_matrix WheelMatrix = RestMatrix;
			WheelMatrix.r[3] = XMVectorSetY(WheelMatrix.r[3],
				XMVectorGetY(WheelMatrix.r[3]) + fSag[i]);

			m_VIBuffer->Set_Transform_Matrix(WHEEL_MESH[iSide][i] + 3, WheelMatrix * ChassisWorld);
		}

		m_VIBuffer->Set_TrackSag(TRACK_MESH[iSide],
			_float4(fSag[0], fSag[1], fSag[2], fSag[3]),
			_float4(fSag[4], fSag[5], fSag[6], 1.f),		/* w = 1 : 셰이더에서 켜기 */
			m_TrackParam[iSide]);
	}
}

/*===========================================================================
	네트워크 보간
===========================================================================*/
namespace
{
	/* 패킷 간격의 허용 범위. 이 밖으로 나가면 순간적인 지터로 보고 잘라낸다. */
	const float NET_INTERVAL_MIN = 1.f / 120.f;
	const float NET_INTERVAL_MAX = 1.f / 15.f;

	/* 한 패킷 사이에 이만큼 튀면 이동이 아니라 순간이동(리스폰 등)으로 본다.
	   차체 길이가 9.6 이므로 그 5배. 최고속으로 달려도 한 틱에 이만큼은 못 간다. */
	const float NET_SNAP_DISTANCE = 50.f;

	/* 각도(도) 보간 - 포탑은 360도를 도므로 359 -> 1 을 짧은 쪽으로 돌려야 한다.
	   안 하면 한 바퀴 거꾸로 휙 도는 게 보인다. */
	float LerpAngleDegree(float fFrom, float fTo, float fRatio)
	{
		float fDiff = fTo - fFrom;

		while (fDiff > 180.f)	fDiff -= 360.f;
		while (fDiff < -180.f)	fDiff += 360.f;

		return fFrom + fDiff * fRatio;
	}
}

void CTank::Push_NetState(const _float4x4& World, float fPotap, float fPosin,
						  bool isChassis, bool isTurret)
{
	m_isNetChassis = isChassis;
	m_isNetTurret  = isTurret;

	FNetSnapshot New;
	New.fPotap = fPotap;
	New.fPosin = fPosin;

	_vector vScale, qRot, vPos;
	if (!XMMatrixDecompose(&vScale, &qRot, &vPos, XMLoadFloat4x4(&World)))
	{
		/* 분해가 안 되는 행렬(스케일 0 등)은 보간할 수 없다. 예전처럼 그대로 넣는다. */
		if (isChassis)
			m_TransformCom->Set_WorldMatrix(World);
		if (isTurret)
		{
			m_fPotapRotation = fPotap;
			m_fPosinRotation = fPosin;
		}
		m_isNetReady = false;
		return;
	}

	XMStoreFloat3(&New.vPos,   vPos);
	XMStoreFloat4(&New.qRot,   qRot);
	XMStoreFloat3(&New.vScale, vScale);

	/* 첫 패킷이거나 순간이동이면 보간하지 않고 그 자리에 놓는다. */
	bool isSnap = !m_isNetReady;

	if (!isSnap && isChassis)
	{
		const float fMoved = XMVectorGetX(XMVector3Length(
			XMLoadFloat3(&New.vPos) - XMLoadFloat3(&m_NetTarget.vPos)));

		if (fMoved > NET_SNAP_DISTANCE)
			isSnap = true;
	}

	if (isSnap)
	{
		m_NetPrev	 = New;
		m_NetTarget	 = New;
		m_NetDisplay = New;
		m_isNetReady = true;
	}
	else
	{
		/*  출발점은 직전 목표 가 아니라 지금 화면에 그려지고 있는 값*/
		m_NetPrev	= m_NetDisplay;
		m_NetTarget	= New;

		/*  실제 도착 간격을 재서 구한다 */
		if (m_fNetSinceRecv >= NET_INTERVAL_MIN)
		{
			const float fMeasured = min(m_fNetSinceRecv, NET_INTERVAL_MAX);
			m_fNetInterval = m_fNetInterval * 0.8f + fMeasured * 0.2f;
		}
	}

	m_fNetElapsed	= 0.f;
	m_fNetSinceRecv	= 0.f;
}

void CTank::Update_NetInterpolation(float fTimeDelta)
{
	if (!m_isNetReady)
		return;

	m_fNetSinceRecv	+= fTimeDelta;
	m_fNetElapsed	+= fTimeDelta;

	float fRatio = (m_fNetInterval > 0.f) ? (m_fNetElapsed / m_fNetInterval) : 1.f;
	if (fRatio > 1.f)
		fRatio = 1.f;			/* 외삽하지 않는다 - 위 주석 참고 */

	/* 회전은 성분을 각각 섞으면 안 된다. 직교성이 깨져 모델이 찌그러진다.
	   쿼터니언 구면보간을 쓴다(짧은 쪽으로 도는 것은 XMQuaternionSlerp 가 처리한다). */
	const _vector vPos = XMVectorLerp(
		XMLoadFloat3(&m_NetPrev.vPos), XMLoadFloat3(&m_NetTarget.vPos), fRatio);
	const _vector qRot = XMQuaternionSlerp(
		XMLoadFloat4(&m_NetPrev.qRot), XMLoadFloat4(&m_NetTarget.qRot), fRatio);
	const _vector vScale = XMVectorLerp(
		XMLoadFloat3(&m_NetPrev.vScale), XMLoadFloat3(&m_NetTarget.vScale), fRatio);

	XMStoreFloat3(&m_NetDisplay.vPos,   vPos);
	XMStoreFloat4(&m_NetDisplay.qRot,   qRot);
	XMStoreFloat3(&m_NetDisplay.vScale, vScale);

	m_NetDisplay.fPotap = LerpAngleDegree(m_NetPrev.fPotap, m_NetTarget.fPotap, fRatio);
	m_NetDisplay.fPosin = LerpAngleDegree(m_NetPrev.fPosin, m_NetTarget.fPosin, fRatio);

	/* 네트워크가 주는 부분만 돌려쓴다. 나머지는 이 클라가 직접 굴리고 있는 값이라
	   덮으면 자기 조작이 서버 왕복에 밀려 뭉개진다. */
	if (m_isNetChassis)
	{
		m_TransformCom->Set_WorldMatrix(
			XMMatrixScalingFromVector(vScale) *
			XMMatrixRotationQuaternion(qRot) *
			XMMatrixTranslationFromVector(vPos));
	}

	if (m_isNetTurret)
	{
		m_fPotapRotation = m_NetDisplay.fPotap;
		m_fPosinRotation = m_NetDisplay.fPosin;
	}
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

	/*  바퀴 14개 + 궤도  */
	const _matrix ChassisWorld = m_TransformCom->Get_WorldMatrix();

	for (int iSide = 0; iSide < 2; ++iSide)
	{
		float fSag[TRACK_WHEEL_COUNT] = {};

		for (_uint i = 0; i < TRACK_WHEEL_COUNT; ++i)
		{
			fSag[i] = DequantizeWheelSag(m_WheelSagWire[iSide * TRACK_WHEEL_COUNT + i]);

			const _uint iMesh = WHEEL_MESH[iSide][i];

			_matrix WheelMatrix = m_VIBuffer->Get_TransformMatrix(iMesh);
			WheelMatrix.r[3] = XMVectorSetY(WheelMatrix.r[3],
				XMVectorGetY(WheelMatrix.r[3]) + fSag[i]);

			m_VIBuffer->Set_Combined_Matrix(iMesh, WheelMatrix * ChassisWorld);
		}

		/* 궤도도 같은 값으로 구부린다(안 하면 바퀴만 움직이고 궤도가 남는다). */
		if (m_isTrackSagReady)
		{
			m_VIBuffer->Set_TrackSag(TRACK_MESH[iSide],
				_float4(fSag[0], fSag[1], fSag[2], fSag[3]),
				_float4(fSag[4], fSag[5], fSag[6], 1.f),
				m_TrackParam[iSide]);
		}
	}

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
	auto sendBuffer = ClientPacketHandler::Make_C_MOVE(TempMat, m_fPotapRotation, m_fPosinRotation, m_WheelSagWire);
	Network_Manager::GetInstance()->Send(sendBuffer);
}

void CTank::SendPosinData() {

	_float4x4 TempMat;
	XMStoreFloat4x4(&TempMat, m_TransformCom->Get_WorldMatrix());
	auto sendBuffer = ClientPacketHandler::Make_C_TANK_POSINMOVE(m_fPotapRotation, m_fPosinRotation);
	Network_Manager::GetInstance()->Send(sendBuffer);
}

void CTank::SendPosData() {

	_float4x4 TempMat;
	XMStoreFloat4x4(&TempMat, m_TransformCom->Get_WorldMatrix());
	auto sendBuffer = ClientPacketHandler::Make_C_TANK_POSMOVE(TempMat, m_WheelSagWire);
	Network_Manager::GetInstance()->Send(sendBuffer);
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
	Network_Manager::GetInstance()->Send(sendBuffer);


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
	/* 남의 탱크 - 차체도 포탑도 전부 서버가 준다. */
	Push_NetState(mat, PotapRot, PosinRot, true, true);
}


void CTank::Set_Posin(float PotapRot, float PosinRot)
{

	Set_PotapRotation(PotapRot);
	Set_PoSinpRotation(PosinRot);

}

void CTank::Set_DriverModeData(float PotapRot, float PosinRot) {
	/* 조종수 - 차체는 자기 PhysX 가 굴리고 포탑만 짝(포수)에게서 온다.
	   차체 인자는 쓰이지 않지만(isChassis=false) 스냅샷을 채워 두기 위해 현재 값을 넘긴다. */
	_float4x4 CurWorld;
	XMStoreFloat4x4(&CurWorld, m_TransformCom->Get_WorldMatrix());

	Push_NetState(CurWorld, PotapRot, PosinRot, false, true);
}


void CTank::Set_MyPos(float x, float y, float z)
{
	m_pPhysicsEngine->Set_Pos(x, y, z);

	_vector safePos = XMVectorSet(x, y, z, 1.0f);
	m_TransformCom->Set_State(CTransform::STATE_POSITION, safePos);


}

void CTank::Set_WheelSagFromServer(const int8* pWheelSag)
{
	if (!pWheelSag)
		return;

	memcpy(m_WheelSagWire, pWheelSag, sizeof(m_WheelSagWire));
}

void CTank::SetMyMatrix(_float4x4 mat) {

	/* 포수 - 포탑은 자기가 돌리고, 자기가 타고 있는 탱크의 '차체' 를 서버에서 받는다.
	   카메라가 이 차체에 붙어 있어서, 보간이 없으면 화면 전체가 60Hz 로 계단이 진다.  */
	Push_NetState(mat, 0.f, 0.f, true, false);
}


void CTank::Tick_For_Posin_Image(float fTimeDelta)
{
	m_RendererCom->AddtoRenderObjects(m_RGQuad, this);

	m_QuadWorldTransform->Identity();

	m_QuadWorldTransform->Set_Scale(CTransform::STATE_UP, 1.7777f);

	// 여기까지는 기본으로 해줘야 하는 것.

	float angleDiffX = m_fPotapRotation - m_fCamPotapRot;
	float angleDiffY = m_fCamPosinRot - m_fPosinRotation;


	m_QuadWorldTransform->Set_Scale(0.05f);

	// 위치 바꾸기
	angleDiffX /= 30.f;
	angleDiffY /= 30.f;

	m_QuadWorldTransform->Set_State(CTransform::STATE_POSITION, XMVectorSet(angleDiffX, angleDiffY, 0, 1));

	m_QuadWorldTransform->Set_State(CTransform::STATE_LOOK, XMVectorSet(0.f, 0.f, 1.f, 0.f));

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
			Network_Manager::GetInstance()->Send(sendBuffer);
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

