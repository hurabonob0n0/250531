#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
END

BEGIN(Client)

class CUI_VICTORY : public CUIObject
{
public:
	CUI_VICTORY();
	CUI_VICTORY(CUI_VICTORY& rhs);
	virtual ~CUI_VICTORY() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

public:
	void set_render() { render = true; }
	void set_render_off() { render = false; }

private:
	CVIBuffer_Quad* m_VIBuffer;

private:
	bool render = false;


public:
	void Free() override;
	static CUI_VICTORY* Create();
	CUI_VICTORY* Clone(void* pArg);

};

END
