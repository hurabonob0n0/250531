#include "UIObject.h"
#include "GameInstance.h"

CUIObject::CUIObject() : CRenderObject()
{
}

CUIObject::CUIObject(CUIObject& rhs) : CRenderObject(rhs)
{
}

HRESULT CUIObject::Initialize_Prototype()
{
	__super::Initialize_Prototype();
	return S_OK;
}

HRESULT CUIObject::Initialize(void* pArg)
{
	__super::Initialize(pArg);

	m_CBBinding = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom");

	return S_OK;
}

void CUIObject::Tick(float fTimeDelta)
{
	if (m_isRender)
		__super::Tick(fTimeDelta);
}

void CUIObject::LateTick(float fTimeDelta)
{
	if (m_isRender)
	{
		__super::LateTick(fTimeDelta);

		m_CBBinding->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);
	}

}

void CUIObject::Render()
{
	if (m_isRender)
		m_CBBinding->Set_On_Shader();
}

void CUIObject::Free()
{
	Safe_Release(m_CBBinding);
	__super::Free();
}

CUIObject* CUIObject::Create()
{
	CUIObject* pInstance = new CUIObject;
	pInstance->Initialize_Prototype();
	return pInstance;
}

CUIObject* CUIObject::Clone(void* pArg)
{
	CUIObject* pInstance = new CUIObject(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}
