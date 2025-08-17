#pragma once
#include "Client_Defines.h"
#include "RenderObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
class CBBinding;
END

BEGIN(Client)

class CEffect : public CRenderObject
{
public:
	CEffect();
	CEffect(CEffect& rhs);
	virtual ~CEffect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Quad* m_VIBuffer;
	CBBinding* m_CBBindingCom;
	CTransform* m_CameraTransform;
	CTransform* m_DroneCamera;

	MaterialData MD{};

private:
	float m_AccumulatedTime = 0.f;

public:
	void Free() override;
	static CEffect* Create();
	CRenderObject* Clone(void* pArg);

};

END