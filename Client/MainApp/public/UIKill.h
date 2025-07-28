#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
END

BEGIN(Client)

class CUIKill : public CUIObject
{
public:
	CUIKill();
	CUIKill(CUIKill& rhs);
	virtual ~CUIKill() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Quad* m_VIBuffer;

private:
	float deltatime;
	float cooltime = 2.f;
	bool isHit = false;

public:
	void reset_deltatime();
	void set_Hit() { isHit = true; }

public:
	void Free() override;
	static CUIKill* Create();
	CUIKill* Clone(void* pArg);

};

END