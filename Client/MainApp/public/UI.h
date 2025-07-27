#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
END

BEGIN(Client)

class CUI : public CUIObject
{
public:
	CUI();
	CUI(CUI& rhs);
	virtual ~CUI() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Quad* m_VIBuffer;
	
private:
	_bool			m_isFPS;

private:
	int TPSMatIndex;
	int FPSMatIndex;
	

public:
	void Free() override;
	static CUI* Create();
	CUI* Clone(void* pArg);

};

END