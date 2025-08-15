#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
END

BEGIN(Client)

class CUIHP : public CUIObject
{
public:
	CUIHP();
	CUIHP(CUIHP& rhs);
	virtual ~CUIHP() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Quad* m_VIBuffer;
	class CUINumber* _100Hp;
	class CUINumber* _10Hp;
	class CUINumber* _1Hp;
	
private:
	int HP = 100;
	class CTransform* m_BaseTransform;
	int FullHP = 100;

public:
	void Free() override;
	static CUIHP* Create();
	CUIHP* Clone(void* pArg);

};

END