#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
END

BEGIN(Client)

class CUISelectPos : public CUIObject
{
public:
	CUISelectPos();
	CUISelectPos(CUISelectPos& rhs);
	virtual ~CUISelectPos() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Quad* m_VIBuffer;
	
private:
	int HP = 100;
	

public:
	void Free() override;
	static CUISelectPos* Create();
	CUISelectPos* Clone(void* pArg);

};

END