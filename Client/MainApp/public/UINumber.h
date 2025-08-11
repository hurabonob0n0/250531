#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
END

BEGIN(Client)

class CUINumber : public CUIObject
{
public:
	CUINumber();
	CUINumber(CUINumber& rhs);
	virtual ~CUINumber() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

public:
	void Set_Number(int num) {
		m_Number = num;
		m_TexCoordTransformCom->Identity();
		__super::Set_TexPos(0.1f * m_Number, 0.f);
		__super::Set_TexScale(0.1f, 1.f);
	}

private:
	CVIBuffer_Quad* m_VIBuffer;
	
private:
	int m_Number = 1;

public:
	void Free() override;
	static CUINumber* Create();
	CUINumber* Clone(void* pArg);

};

END