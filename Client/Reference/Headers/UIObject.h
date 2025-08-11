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

public:
	void Set_Pos(float x, float y) {
		float tx = (x - 960.f) / 960.f;
		float ty = (y - 540.f) / 540.f;
		m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(tx, ty, 0.f, 1.f));
	}
	void Set_Scale(float x, float y)
	{
		m_TransformCom->Set_Scale(CTransform::STATE_RIGHT, x);
		m_TransformCom->Set_Scale(CTransform::STATE_UP, y);
	}
	void Set_TexPos(float x, float y)
	{
		m_TexCoordTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(x, y, 0.f, 1.f));
	}
	void Set_TexScale(float x, float y)
	{
		m_TexCoordTransformCom->Set_Scale(CTransform::STATE_RIGHT, x);
		m_TexCoordTransformCom->Set_Scale(CTransform::STATE_UP, y);
	}
	void Set_isRender(bool RenderOrNot) { m_isRender = RenderOrNot; }

protected:
	CBBinding* m_CBBinding;
	bool m_isRender = true;

public:
	void Free() override;
	static CUIObject* Create();
	CUIObject* Clone(void* pArg);
};

END