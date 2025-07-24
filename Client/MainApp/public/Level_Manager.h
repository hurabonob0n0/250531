#pragma once
#include "Level_Menu.h"
#include "Level_RoomList.h"
#include "Level_Room.h"

class Level_Manager
{
private:
	Level_Manager();
	~Level_Manager();

public:
	void Level_Change(LEVEL_ID eID);

	LEVEL_ID GetSceneID() {
		return m_eCurLevel;
	}
	int	 Update();
	void Late_Update();
	void Render(HDC hDC);
	void Release(void);


public:
	static	Level_Manager* Get_Instance()
	{
		if (!m_pInstance)
		{
			m_pInstance = new Level_Manager;
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

public:
	bool GoToGamePlay() {
		return GoPlay;
	};
	void SetGamePlayMode() { GoPlay = true; };

	void Ready_EnterRoom() {
		EnterRoom = true;
	}
	
	void Ready_ExitRoom() {

		ExitRoom = true;
	}


private:
	static	Level_Manager*			m_pInstance;
	Level*							m_pLevel = nullptr;
	LEVEL_ID						m_ePreLevel;
	LEVEL_ID						m_eCurLevel;

	
	bool EnterRoom = false;
	bool ExitRoom = false;
	bool GoPlay = false;
};

