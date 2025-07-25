#include "Client_pch.h"
#include "Level_Manager.h"


Level_Manager* Level_Manager::m_pInstance = nullptr;

Level_Manager::Level_Manager() : m_eCurLevel(LEVEL_MENU), m_ePreLevel(LEVEL_END)
{
}

Level_Manager::~Level_Manager()
{
	Release();
}

void Level_Manager::Level_Change(LEVEL_ID eID)
{
	m_eCurLevel = eID;

	if (m_eCurLevel != m_ePreLevel) {
		Safe_Delete(m_pLevel);

		switch (m_eCurLevel) {
		case LEVEL_MENU:
			m_pLevel = new Level_Menu;
			break;
		case LEVEL_ROOMLIST:
			m_pLevel = new Level_RoomList;
			break;
		case LEVEL_ROOM:
			m_pLevel = new Level_Room;
			break;
		default:
			break;

		}

		m_pLevel->Initialize();
		m_ePreLevel = m_eCurLevel;
	}
}

int Level_Manager::Update()
{
	if (EnterRoom) {
		Level_Change(LEVEL_ROOM);
		EnterRoom = false;
	}
	if (ExitRoom) {


	}

	m_pLevel->Update();
	return 0;
}

void Level_Manager::Late_Update()
{
	m_pLevel->Late_Update();
}

void Level_Manager::Render(HDC hDC)
{
	m_pLevel->Render(hDC);
}

void Level_Manager::Release(void)
{
	Safe_Delete(m_pLevel);
}

