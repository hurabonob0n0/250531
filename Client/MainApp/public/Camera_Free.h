#pragma once
#include "Client_Defines.h"
#include "Camera.h"
#include "Tank.h"

BEGIN(Engine)
class CVIBuffer_Geos;
class CBBinding;
END

BEGIN(Client)

class CCamera_Free : public CCamera
{
	enum PERSPECTIVE{FPS,TPS,PS_END};

public:
	CCamera_Free();
	CCamera_Free(CCamera_Free& rhs);
	virtual ~CCamera_Free() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	void PassCBSetting();

private:
	void Tick_For_TPS(float fTimeDelta);
	void Tick_For_FPS(float fTimeDelta);


private:
	CVIBuffer_Geos* m_VIBuffer;
	CBBinding* m_CBBindingCom;

	CTransform* m_TankTransform;
	CTank*		m_Tank;


	float		m_fYRot_TPS;
	float		m_fXRot_TPS;
	float		m_Distance_TPS = 60.f;

	float		m_fYRot_FPS;
	float		m_fXRot_FPS;
	float		m_Distance_FPS = 60.f;

	class CTerrain* m_Terrain;
	
	PERSPECTIVE m_PS = TPS;


public:

	void CCamera_Free::StartShake(float duration, float amplitude, float frequency);
	void CCamera_Free::ApplyCameraShake(float dt);

private:

	bool   m_IsShaking = false;     // 셰이크 중 여부(중복 호출 방지)
	float  m_ShakeTime = 0.f;       // 진행 시간
	float  m_ShakeDur = 0.f;       // 총 지속
	float  m_ShakeAmp = 0.f;       // 진폭(카메라-공간 오프셋, 단위: world units)
	float  m_ShakeFreq = 0.f;       // 주파수(진동 속도)
	float  m_ShakeSeed = 0.f;       // 위상 시드(개별 호출마다 달리)

	DirectX::XMFLOAT3 m_ShakeOfs = { 0,0,0 };

public:
	void Free() override;
	static CCamera_Free* Create();
	CCamera_Free* Clone(void* pArg);


};

END