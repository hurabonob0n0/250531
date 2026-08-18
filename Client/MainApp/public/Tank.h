#pragma once
#include "RenderObject.h"
#include "MyPhysicsEngine.h"
#include <functional>

BEGIN(Engine)
class CModel;
class CBBinding;
class CVIBuffer_Quad;
END

namespace FMOD { class Channel; }
class FMOD_Manager;

BEGIN(Client)

class CTank : public CRenderObject
{
private:
	CTank();
	CTank(CTank& rhs);
	virtual ~CTank() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

public:
	void Set_Team(int Team);

public:
	void Set_PotapRotation(float fDegree);
	void Set_PoSinpRotation(float fDegree);
	void Set_Other_PotapRotation(float fDegree);
	void Set_Other_PoSinpRotation(float fDegree);
	void Set_Potap_For_Driver(float fDegree);
	void Set_Posin_For_Driver(float fDegree);
	void Set_ShotDir(XMVECTOR Vec);
	void Set_ShotMatrix(_matrix mat);

	void Respawn();
private:
	CModel* m_VIBuffer;

	_matrix ShotMatrix;
	XMVECTOR vShotDir;
	float   m_fPotapRotation = 0.f;			//이게 카메라가 주는 회전값
	float   m_fPosinRotation = 0.f;			//이게 카메라가 주는 회전값
	float	m_fCamPotapRot = 0.f;
	float	m_fCamPosinRot = 0.f;

	float   m_TestZ = -512.f;
	float	m_TestX = -512.f;
	bool	KeyInput = false;

	_uint matindex = 0;

private:
	float fTest = 1.f;

private:
	void RotPotap_And_Posin(float fTimeDelta);

public:
	//For AirDrop
	void  UpdateAirDropCooldown(float dt);
	bool  CanEnterAirDropMode() const;
	void  Start_AirDropMode();     // UI on
	void  Exit_AirDropMode();      // UI off
	void  Request_Air_Drop();      // 숫자키 처리(보내면 쿨타임 시작)
	void  Enter_Air_DropMode();    // (키 입력으로 진입 시 호출)

	static constexpr float AIRDROP_COOLDOWN_SEC = 30.f;

	bool  _airdropMode = false;  // 선택 UI/상태 on/off
	bool  _airdropReady = true;   // 쿨타임 여부
	float _airdropTimer = 0.f;    // 경과 시간

	float GetCoolScale() {
		if (_airdropReady)
			return 0;

		// 스킬 직후(타이머 0초) → 1,  
		// 시간이 지날수록 0에 가까워지게
		return 1.0f - (_airdropTimer / AIRDROP_COOLDOWN_SEC);
	};

public:
	/*--------------
	
		For 함수화

	-------------*/

	void Master_Pos_KeyInput();
	void Driver_Pos_KeyInput();
	void POSU_Pos_KeyInput();

	void Set_Tank_Element_from_Engine();
	void Set_Tank_Element_from_ServerData();

	void ErrorRespawn();
public:
	void set_MyPlayer() {
		_myPlayer = true;
		Set_Team(3);
	}
	void Set_MyTeam(bool isBlue) { _MyTeam = isBlue; };
	void SendMyStateToServer();
	void SendPosinData();
	void SendPosData();
	void SendShootDataToServer();

	void Set_OtherPlayerState(_float4x4 mat, float PotapRot, float PosinRot);


	void Set_Posin(float PotapRot, float PosinRot);

	void Set_DriverModeData(float PotapRot, float PosinRot);

	void Set_MyPos(float x, float y, float z);

	void SetMyMatrix(_float4x4 mat);

	/*  서버가 준 바퀴 서스펜션(int8 14개)을 받아둔다.           */
	void Set_WheelSagFromServer(const int8* pWheelSag);

private:
 
	struct FNetSnapshot
	{
		_float3	vPos	= { 0.f, 0.f, 0.f };
		_float4	qRot	= { 0.f, 0.f, 0.f, 1.f };
		_float3	vScale	= { 1.f, 1.f, 1.f };
		float	fPotap	= 0.f;
		float	fPosin	= 0.f;
	};

	FNetSnapshot	m_NetPrev;		/* 출발 - 목표를 받은 순간 화면에 그려지고 있던 값 */
	FNetSnapshot	m_NetTarget;	/* 도착 - 가장 최근에 받은 값 */
	FNetSnapshot	m_NetDisplay;	/* 이번 프레임에 실제로 그린 값 */

	float	m_fNetElapsed	= 0.f;	/* 목표를 받은 뒤 흐른 시간 */
	float	m_fNetInterval	= 1.f / 60.f;	/* 최근 패킷 간격(평활). 이 시간에 걸쳐 이동한다 */
	float	m_fNetSinceRecv	= 0.f;	/* 간격 측정용 */
	bool	m_isNetReady	= false;	/* 첫 패킷은 보간 없이 그 자리에 놓는다 */
	bool	m_isNetChassis	= false;	/* 차체를 네트워크에서 받는가(포수/원격 탱크) */
	bool	m_isNetTurret	= false;	/* 포탑·포신을 네트워크에서 받는가(조종수/원격 탱크) */

	void Push_NetState(const _float4x4& World, float fPotap, float fPosin,
					   bool isChassis, bool isTurret);
	void Update_NetInterpolation(float fTimeDelta);

	/*  송신 주기를 렌더 프레임에서 떼어낸다   */
	float	m_fNetSendTimer = 0.f;

	/*  이 탱크의 바퀴 서스펜션을 int8 로 낮춰 담아둔 것.  */
	int8	m_WheelSagWire[TANK_WHEEL_COUNT] = {};

private:		//For PosinCrosshair
	CBBinding* m_CBBindingQuad;
	CVIBuffer_Quad* m_VIBufferQuad;
	CRenderer::RENDERGROUP	m_RGQuad = CRenderer::RENDERGROUP::RG_UI;
	CTransform* m_QuadWorldTransform;
	CTransform* m_QuadTexTransform;
	_uint m_isQuadTurn = 0;
	float					m_fSameTime = 0.f;
	bool					m_isFPS = false;


private:		//For PosinCrosshair
	void Initialize_For_PosinQuad();
	void Tick_For_Posin_Image(float fTimeDelta);
	void Render_For_Posin_Image();

	void CheckRespawnKeyInput();
public:
	void setRespawn();
	void setRespawnForPosinMode();
private:
	float _shootTimer = 0.f;  // 누적 슈팅 시간
	const float SHOOT_INTERVAL = 3.f; // 4초 주기

public:
	bool _myPlayer = false;
	bool _MyTeam;
	bool is_RespawnArea_choiced;
	bool is_AirDropArea_choiced = false;

	int Choiced_Pos;
	bool _isSpawn;
	void set_Spawn(bool torf) { _isSpawn = torf; };
	float _respawnTimer = 0.0f;
private:
	MyPhysicsEngine::CMyPhysicsEngine* m_pPhysicsEngine = nullptr;
	MyPhysicsEngine::CMyPhysicsEngine::TankControlState m_TankConsrolState{};

private:
	/*------------------------------------------------------------------
		궤도를 로드휠 서스펜션에 맞춰 구부리기

		궤도 메쉬(38 왼쪽, 39 오른쪽)는 차체에 고정돼 있어서, 예전에는 바퀴가
		위아래로 움직여도 궤도는 가만히 있었다. 바퀴가 쉴 때 위치에서 얼마나
		움직였는지를 재서 셰이더(Shaders1/Common.hlsl 의 TrackSag)로 넘긴다.
		실제로 정점을 옮기는 건 GPU 라서 CPU 비용도 메모리 비용도 없다.
	------------------------------------------------------------------*/
	void Ready_Track_Sag();
	void Update_Wheels_And_Track(_fmatrix ChassisWorld, const _matrix* pPhysXWheelMatrices);

	static const _uint TRACK_WHEEL_COUNT = 7;

	bool		m_isTrackSagReady = { false };
	_float4x4	m_WheelRestMatrix[2][TRACK_WHEEL_COUNT] = {};	/* 바퀴가 쉴 때의 모델공간 행렬 */
	_float4		m_TrackParam[2] = {};							/* 셰이더로 넘길 궤도 구간 정보 */

public:
	void PushBulletMatrix(const _matrix& mat);
	void PopAllBulletMatrix(std::function<void(const _matrix&)> processFunc);

private:
	std::queue<_matrix> BulletQueue;
	std::mutex BulletQueueMutex;

public:
	void Free() override;
	static CTank* Create();
	CRenderObject* Clone(void* pArg);




	/*for Sound*/
public:

	void OnSpawnAudio();           // 루프 재생 시작(채널 생성)
	void OnDespawnAudio();         // 루프 정리
	void UpdateAudio(float dt);    // 매 프레임 볼륨/피치/3D 업데이트
	void SetRpmInput01(float v) { _rpmInput = v < 0 ? 0.f : (v > 1 ? 1.f : v); }
	void SetIsMoving(bool b) { _isMoving = b; }

	void SetSoundData(float engVol, float engPit, float trkVol, float trkPit);

private:
	FMOD::Channel* _engineCh = nullptr; // 엔진 루프
	FMOD::Channel* _trackCh = nullptr; // 궤도 루프
	float _rpmInput = 0.f;              // 0~1, 키 인풋 기반 목표값
	float _rpmSm = 0.f;              // 0~1, 스무딩된 RPM
	bool  _isMoving = false;            // 움직임 여부(키 인풋 기반)

	// [튜닝값] 필요 시 조정
	const float _rpmRise = 1.7f;      // RPM 상승 속도(초당)
	const float _rpmFall = 2.0f;      // RPM 하강 속도(초당)
	const float _engineVolBase = 0.35f; // 엔진 최소 볼륨
	const float _engineVolGain = 0.45f; // 엔진 RPM 볼륨 가산
	const float _enginePitchLo = 0.95f; // 엔진 피치 하한
	const float _enginePitchHi = 1.30f; // 엔진 피치 상한
	const float _trackVolBase = 0.0f;  // 궤도 최소 볼륨(정지=0)
	const float _trackVolGain = 0.9; // 궤도 RPM 볼륨 가산
	const float _trackPitchLo = 0.95f; // 궤도 피치 하한
	const float _trackPitchHi = 1.15f; // 궤도 피치 상한
	float _trackMixGain = 0.3f;


	bool  _useNetMix = false;
	float _netEngVol = 0.f;
	float _netEngPit = 1.f;
	float _netTrkVol = 0.f;
	float _netTrkPit = 1.f;
	float _netHoldT = 0.f;
};

END