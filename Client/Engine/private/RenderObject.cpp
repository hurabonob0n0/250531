#include "RenderObject.h"
#include "GameInstance.h"
#include "VIBuffer_Geos.h"


CRenderObject::CRenderObject() : CGameObject()
{
}

CRenderObject::CRenderObject(CRenderObject& rhs) : CGameObject(rhs)
{
}


void CRenderObject::Set_RenderGroup(CRenderer::RENDERGROUP RG)
{
	m_RG = RG;
}

HRESULT CRenderObject::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CRenderObject::Initialize(void* pArg)
{
	__super::Initialize(pArg);

	m_RendererCom = (CRenderer*)m_GameInstance->Get_Component("RendererCom", nullptr);

	m_TexCoordTransformCom = (CTransform*)m_GameInstance->Get_Component("TransformCom", nullptr);

	return S_OK;
}

void CRenderObject::Tick(float fTimeDelta)
{
}

void CRenderObject::LateTick(float fTimeDelta)
{
	m_RendererCom->AddtoRenderObjects(m_RG, this);

}

void CRenderObject::Render()
{
	
}

void CRenderObject::Free()
{
	Safe_Release(m_RendererCom);
	Safe_Release(m_TexCoordTransformCom);
	__super::Free();
}

CRenderObject* CRenderObject::Create()
{
	CRenderObject* pInstance = new CRenderObject;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CRenderObject* CRenderObject::Clone(void* pArg)
{
	CRenderObject* pInstance = new CRenderObject(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
