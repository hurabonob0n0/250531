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

public:
	void Set_Reloading() { reloading = true; }

private:
	CVIBuffer_Quad* m_VIBuffer;
	
private:
	bool reloading = false;
	float deltatime = 0.f;
	float cooltime = 3.f;
	

public:
	void Free() override;
	static CUIReloading* Create();
	CUIReloading* Clone(void* pArg);

};

END