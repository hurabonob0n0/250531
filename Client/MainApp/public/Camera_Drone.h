#pragma once
#include "Client_Defines.h"
#include "Camera.h"
#include "Tank.h"

BEGIN(Engine)
class CVIBuffer_Geos;
class CBBinding;
END

BEGIN(Client)

class CCamera_Drone : public CCamera
{
	enum PERSPECTIVE{FPS,TPS,PS_END};

public:
	CCamera_Drone();
	CCamera_Drone(CCamera_Drone& rhs);
	virtual ~CCamera_Drone() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	void Tick_For_TPS(float fTimeDelta);
	void Tick_For_FPS(float fTimeDelta);


private:
	CVIBuffer_Geos* m_VIBuffer;
	CBBinding* m_CBBindingCom;

	CTransform* m_DroneTransform;

	float		m_fYRot_TPS;
	float		m_fXRot_TPS;
	float		m_Distance_TPS;

	class CTerrain* m_Terrain;
	
	PERSPECTIVE m_PS = TPS;

	bool isRotation = true;

	bool isDroneRender = false;

	_float4 LastPos;
public:
	void SetDroneRender(bool tf) {

		isDroneRender = tf;
	}
public:
	void Free() override;
	static CCamera_Drone* Create();
	CCamera_Drone* Clone(void* pArg);
};

END