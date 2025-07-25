#include "Client_pch.h"
#include "Object_Manager.h"

Object_Manager* Object_Manager::m_pInstance = nullptr;

Object_Manager::Object_Manager()
{
}

Object_Manager::~Object_Manager()
{
	Release();
}

void Object_Manager::Add_Object(OBJ_ID eID, GameObject* pGameObject)
{
	if (OBJ_END <= eID || nullptr == pGameObject)
		return;

	m_ObjectList[eID].push_back(pGameObject);
}

int Object_Manager::Update(void)
{

	for (size_t i = 0; i < OBJ_END; ++i)
	{
		for (auto iter = m_ObjectList[i].begin();
			iter != m_ObjectList[i].end(); )
		{
			int iResult = (*iter)->Update();

			if (LevelChange)
				return 0;

			if (OBJ_DEAD == iResult)
			{
				Safe_Delete<GameObject*>(*iter);
				iter = m_ObjectList[i].erase(iter);
			}
			else
				++iter;
		}
	}

	return 0;
}

void Object_Manager::Late_Update(void)
{
	for (size_t i = 0; i < OBJ_END; ++i)
	{
		for (auto& iter : m_ObjectList[i])
		{
			iter->Late_Update();
			if (LevelChange)
				return;

			if (m_ObjectList[i].empty())
				break;

		}
	}

}

void Object_Manager::Render(HDC hDC)
{
	for (size_t i = 0; i < OBJ_END; ++i)
	{
		for (auto& iter : m_ObjectList[i])
		{
			iter->Render(hDC);
		}
	}
}

void Object_Manager::Release(void)
{

	for (size_t i = 0; i < OBJ_END; ++i)
	{
		for_each(m_ObjectList[i].begin(), m_ObjectList[i].end(), Safe_Delete<GameObject*>);
		m_ObjectList[i].clear();
	}
}

void Object_Manager::DeleteID(OBJ_ID eID)
{
	for (auto& iter : m_ObjectList[eID])
		Safe_Delete(iter);

	m_ObjectList[eID].clear();
}
