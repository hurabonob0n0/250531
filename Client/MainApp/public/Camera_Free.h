#pragma once
#include "Client_Defines.h"
#include "Camera.h"

BEGIN(Engine)
class CVIBuffer_Geos;
class CBBinding;
END

BEGIN(Client)

class CCamera_Free : public CCamera
{
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
	CVIBuffer_Geos* m_VIBuffer;
	CBBinding* m_CBBindingCom;

public:
	void Free() override;
	static CCamera_Free* Create();
	CCamera_Free* Clone(void* pArg);
};

END