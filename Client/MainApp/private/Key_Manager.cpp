#include "Client_pch.h"
#include "Key_Manager.h"

Key_Manager* Key_Manager::m_pInstance = nullptr;

Key_Manager::Key_Manager()
{
	ZeroMemory(m_bKeyState, sizeof(m_bKeyState));
}

Key_Manager::~Key_Manager()
{
}

bool Key_Manager::Key_Pressing(int _iKey)
{
	if (GetAsyncKeyState(_iKey) & 0x8000)
		return true;

	return false;
}

bool Key_Manager::Key_Down(int _iKey)
{
	if (!m_bKeyState[_iKey] && (GetAsyncKeyState(_iKey) & 0x8000))
	{
		m_bKeyState[_iKey] = !m_bKeyState[_iKey];
		return true;
	}

	for (int i = 0; i < VK_MAX; ++i)
	{
		if (m_bKeyState[i] && !(GetAsyncKeyState(_iKey) & 0x8000))
			m_bKeyState[i] = !m_bKeyState[i];
	}

	return false;
}

bool Key_Manager::Key_Up(int _iKey)
{
	if (m_bKeyState[_iKey] && !(GetAsyncKeyState(_iKey) & 0x8000))
	{
		m_bKeyState[_iKey] = !m_bKeyState[_iKey];
		return true;
	}

	for (int i = 0; i < VK_MAX; ++i)
	{
		if (!m_bKeyState[i] && (GetAsyncKeyState(_iKey) & 0x8000))
			m_bKeyState[i] = !m_bKeyState[i];
	}



	return false;
}
