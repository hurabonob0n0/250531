#include "GameInstance.h"
#include "GameObject.h"
#include "Transform.h"

CGameObject::CGameObject()
{
}

CGameObject::CGameObject(CGameObject& rhs)
{
}

HRESULT CGameObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGameObject::Initialize(void* pArg)
{
	m_GameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(m_GameInstance);
	m_TransformCom = (CTransform*)m_GameInstance->Get_Component("TransformCom", pArg);

	return S_OK;
}

void CGameObject::Free()
{
	Safe_Release(m_GameInstance);
	Safe_Release(m_TransformCom);
}
