#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
END

BEGIN(Client)

class CUICompass : public CUIObject
{
public:
	CUICompass();
	CUICompass(CUICompass& rhs);
	virtual ~CUICompass() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Quad* m_VIBuffer;
	CTransform* m_CameraFree;
	CTransform* m_DroneCamera;

public:
	void Free() override;
	static CUICompass* Create();
	CUICompass* Clone(void* pArg);

};

END
