#pragma once
#include "RenderObject.h"
#include "MyPhysicsEngine.h"
#include <functional>

BEGIN(Engine)
class CModel;
class CBBinding;
class CVIBuffer_Quad;
END


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

	static constexpr float AIRDROP_COOLDOWN_SEC = 10.f;

	bool  _airdropMode = false;  // 선택 UI/상태 on/off
	bool  _airdropReady = true;   // 쿨타임 여부
	float _airdropTimer = 0.f;    // 경과 시간


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

private:		//For PosinCrosshair
	CBBinding* m_CBBindingQuad;
	CVIBuffer_Quad* m_VIBufferQuad;
	CRenderer::RENDERGROUP	m_RGQuad = CRenderer::RENDERGROUP::RG_UI;
	CTransform* m_QuadWorldTransform;
	CTransform* m_QuadTexTransform;
	bool					m_isQuadTurn = false;
	float					m_fSameTime = 0.f;
	bool					m_isFPS = false;


private:		//For PosinCrosshair
	void Initialize_For_PosinQuad();
	void Tick_For_Posin_Image(float fTimeDelta);
	void Render_For_Posin_Image();

	void CheckRespawnKeyInput();

	void setRespawn();
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
};

END