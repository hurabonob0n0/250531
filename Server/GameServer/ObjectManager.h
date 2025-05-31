#pragma once
#include "GameObject.h"

class ObjectManager
{
	//룸 마다 오브젝트 매니저 하나씩 가지고 있기
private:
	ObjectManager();
	~ObjectManager();

public:
	
	void Add_Object(ObjectID eID, GameObject* pGameObject) {
		if (pGameObject)
			m_ObjectList[eID].push_back(pGameObject);
	}

	vector<GameObject*>* Get_List(ObjectID eID) {
		if (!List_Empty(eID))
			return &m_ObjectList[eID];
		return nullptr;
	}

	bool List_Empty(ObjectID eID) {
		return m_ObjectList[eID].empty();
	}

public:
	static	ObjectManager* Get_Instance()
	{
		if (!m_pInstance)
		{
			m_pInstance = new ObjectManager;
		}
		return m_pInstance;
	}

	static void			Destroy_Instance()
	{
		if (m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = nullptr;
		}
	}

	int Update();
	void Late_Update();
	void Release();

private:
	static	ObjectManager* m_pInstance;

	std::vector<GameObject*> m_ObjectList[OBJ_END];


};

