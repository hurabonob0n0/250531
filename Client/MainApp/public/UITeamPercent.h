#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
END

BEGIN(Client)

class CUIReloading : public CUIObject
{
public:
	CUIReloading();
	CUIReloading(CUIReloading& rhs);
	virtual ~CUIReloading() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Quad* m_VIBuffer;
	CBBinding* m_CBBinding1;
	
private:
	bool reloading = true;
	

public:
	void Free() override;
	static CUIReloading* Create();
	CUIReloading* Clone(void* pArg);

};

END