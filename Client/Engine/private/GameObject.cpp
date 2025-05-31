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
	m_Components.insert(std::make_pair("TransformCom", m_TransformCom));

	return S_OK;
}

void CGameObject::Free()
{
	for (auto& pair : m_Components)
	{
		Safe_Release(pair.second);
	}
	Safe_Release(m_GameInstance);
	Safe_Release(m_TransformCom);
}
