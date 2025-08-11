#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
END

BEGIN(Client)

class CUICrossHair : public CUIObject
{
public:
	CUICrossHair();
	CUICrossHair(CUICrossHair& rhs);
	virtual ~CUICrossHair() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

public:
	void Update_State();

private:
	CVIBuffer_Quad* m_VIBuffer;

private:
	int TPSMatIndex;
	int FPSMatIndex;
	int DroneMatIndex;
	

public:
	void Free() override;
	static CUICrossHair* Create();
	CUICrossHair* Clone(void* pArg);

};

END