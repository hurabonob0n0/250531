#pragma once
#include "GameObject.h"
#include "Renderer.h"
#include "Transform.h"

BEGIN(Engine)

class CVIBuffer;

class ENGINE_DLL CRenderObject : public CGameObject
{
public:
	CRenderObject();
	CRenderObject(CRenderObject& rhs);
	virtual ~CRenderObject() = default;

public:
	virtual void Set_RenderGroup(CRenderer::RENDERGROUP RG);
	_matrix	Get_WorldMatrix() { return m_TransformCom->Get_WorldMatrix(); }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

protected:
	CRenderer* m_RendererCom = {};
	CTransform* m_TexCoordTransformCom = {};

protected:
	CRenderer::RENDERGROUP m_RG = CRenderer::RENDERGROUP::RG_PRIORITY;

public:
	void Free() override;
	static CRenderObject* Create();
	CRenderObject* Clone(void* pArg);
};

END