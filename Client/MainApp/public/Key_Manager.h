#pragma once
#include "Define.h"

class Key_Manager
{
private:
	Key_Manager();
	~Key_Manager();

public:
	bool		Key_Pressing(int _iKey);
	bool		Key_Down(int _iKey);
	bool		Key_Up(int _iKey);

public:
	static	Key_Manager* Get_Instance()
	{
		if (!m_pInstance)
		{
			m_pInstance = new Key_Manager;
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
	static	Key_Manager* m_pInstance;
	bool		m_bKeyState[VK_MAX];
};

