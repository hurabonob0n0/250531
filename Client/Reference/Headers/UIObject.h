#pragma once
#include "RenderObject.h"

BEGIN(Engine)

class CBBinding;

class ENGINE_DLL CUIObject : public CRenderObject
{
public:
	CUIObject();
	CUIObject(CUIObject& rhs);
	virtual ~CUIObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

protected:
	CBBinding* m_CBBinding;

public:
	void Free() override;
	static CUIObject* Create();
	CUIObject* Clone(void* pArg);
};

END