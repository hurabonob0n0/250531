#pragma once
#include "Client_Defines.h"
#include "RenderObject.h"

BEGIN(Engine)
class CVIBuffer_Geos;
class CBBinding;
END

BEGIN(Client)

class CDefaultObj : public CRenderObject
{
public:
	CDefaultObj();
	CDefaultObj(CDefaultObj& rhs);
	virtual ~CDefaultObj() = default;

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
	static CDefaultObj* Create();
	CRenderObject* Clone(void* pArg);

};

END