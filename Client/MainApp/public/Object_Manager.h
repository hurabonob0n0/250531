#pragma once
#include "GameObject.h"

class Object_Manager
{

	Object_Manager();
	~Object_Manager();

public:
	void		Add_Object(OBJ_ID eID, GameObject* pGameObject);
	int			Update(void);
	void		Late_Update(void);
	void		Render(HDC hDC);
	void		Release(void);


	list<GameObject*>* Get_List(OBJ_ID eID) {
		if (!List_Empty(eID))
			return &m_ObjectList[eID];
	}

	bool List_Empty(OBJ_ID eID) {
		if (m_ObjectList[eID].empty() == true) {
			return true;
		}
		else {
			return false;
		}

	};

	void		DeleteID(OBJ_ID eID);

private:

	list<GameObject*>		m_ObjectList[OBJ_END];

public:
	static	Object_Manager* Get_Instance()
	{
		if (!m_pInstance)
		{
			m_pInstance = new Object_Manager;
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
private:
	static	Object_Manager* m_pInstance;

public:
	bool LevelChange = false;
};

