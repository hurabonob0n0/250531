#pragma once
#include "RenderObject.h"

BEGIN(Engine)

class ENGINE_DLL CCamera : public CRenderObject
{
public:
	CCamera();
	CCamera(CCamera& rhs);
	virtual ~CCamera() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	bool m_isPaused = false;
	float m_Speed = 1.f;

public:
	static CCamera* Create();
	CCamera* Clone(void* pArg);
	void Free() override;
};

END