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
	void Tick_For_TPS(float fTimeDelta);
	void Tick_For_FPS(float fTimeDelta);


private:
	CVIBuffer_Geos* m_VIBuffer;
	CBBinding* m_CBBindingCom;

	CTransform* m_TankTransform;
	CTank*		m_Tank;


	float		m_fYRot_TPS;
	float		m_fXRot_TPS;
	float		m_Distance_TPS;

	float		m_fYRot_FPS;
	float		m_fXRot_FPS;
	float		m_Distance_FPS;
	
	PERSPECTIVE m_PS = TPS;
public:
	void Free() override;
	static CCamera_Free* Create();
	CCamera_Free* Clone(void* pArg);
};

END