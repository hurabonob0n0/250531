#pragma once
#include "RenderObject.h"
#include "UINumber.h"

BEGIN(Client)

class CUITime : public CRenderObject
{
public:
	CUITime();
	CUITime(CUITime& rhs);
	virtual ~CUITime() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

public:
	void Update_Time(float fTimeDelta);

private:
	float fGameTime = 0.f;
	bool Active = false;

private:
	CUINumber* TenMinutes;
	CUINumber* OneMinutes;
	CUINumber* TenSeconds;
	CUINumber* OneSeconds;

private:
	_float2 TenMinutePos;
	_float2 OneMinutePos;
	//_float2 

public:
	void Free() override;
	static CUITime* Create();
	CUITime* Clone(void* pArg);
};

END