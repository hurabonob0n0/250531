#pragma once
#include "RenderObject.h"

BEGIN(Engine)
class CVIBuffer_Terrain;
class CBBinding;
END

BEGIN(Client)
class CTerrain : public CRenderObject
{
private:
	CTerrain();
	CTerrain(CTerrain& rhs);
	virtual~CTerrain() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Terrain* m_VIBufferCom;
	CBBinding* m_CBBindingCom;


public:
	static CTerrain* Create();
	CRenderObject* Clone(void* pArg);
	virtual void Free() override;
};

END