#pragma once
#include "Client_Defines.h"
#include "RenderObject.h"

BEGIN(Engine)
class CVIBuffer_Geos;
END

BEGIN(Client)

class CBoxObj : public CRenderObject
{
public:
	CBoxObj();
	CBoxObj(CBoxObj& rhs);
	virtual ~CBoxObj() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Geos* m_VIBuffer;

public:
	void Free() override;
	static CBoxObj* Create();
	CRenderObject* Clone(void* pArg);

};

END