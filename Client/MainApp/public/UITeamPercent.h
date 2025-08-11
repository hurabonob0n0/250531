#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
END

BEGIN(Client)

class CUITeamPercent : public CUIObject
{
public:
	CUITeamPercent();
	CUITeamPercent(CUITeamPercent& rhs);
	virtual ~CUITeamPercent() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Quad* m_VIBuffer;

	_float2 BlankPos{ 960,980.f };
	_float2 BlankScale{ 1.f,0.1f };
	_float2 BlueTeamPos{ 700.f,965.f };
	_float2 BlueTeamScale{0.45f,0.025f };
	_float2 RedTeamPos{ 1220.f,965.f };
	_float2 RedTeamScale{0.45f,0.025f };

private:
	int BlankMatIndex;
	int BlueMatIndex;
	int RedMatIndex;
	

public:
	void Free() override;
	static CUITeamPercent* Create();
	CUITeamPercent* Clone(void* pArg);

};

END