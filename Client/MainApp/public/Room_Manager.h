#pragma once
class Room_Manager
{
private:

	Room_Manager();
	~Room_Manager();

public:
	static	Room_Manager* Get_Instance()
	{
		if (!m_pInstance)
		{
			m_pInstance = new Room_Manager;
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

	const std::vector<Room_Data>& GetRoomList() const
	{
		return vRoom_List;
	}

	void SetRoomList(const std::vector<Room_Data>& newList)
	{
		vRoom_List = newList;
		isChange = true;
	}


	void AddRoom(const Room_Data& room)
	{
		vRoom_List.push_back(room);
	}

	void ClearRoomList()
	{
		vRoom_List.clear();
	}

	bool isChange = false;

	void SetRoomPlayerStates(const std::vector<Room_Ready_Data>& playerStates) { vRoom_Player_Data = playerStates; }
	void ClearRoomPlayerStates() { vRoom_Player_Data.clear(); }


	const std::vector<Room_Ready_Data>& GetRoomPlayerStates() const
	{
		return vRoom_Player_Data;
	}

	void ChoiceRoom(int ID) { ChoiceRoomNum = ID; };

	int GetChoiceRoom() { return ChoiceRoomNum; };

	void	SetCurRoom(int RoomID) { CurRoom = RoomID; };
	int		GetCurRoom() { return CurRoom; };

	void    ClearCurRoom() { CurRoom = ROBBY; };


	void SetMyPlayerData(const Room_Ready_Data& data) { myPlayerData = data; }

	const Room_Ready_Data& GetMyPlayerData() const { return myPlayerData; }

private:
	static	Room_Manager* m_pInstance;

	std::vector<Room_Data> vRoom_List;
	std::vector<Room_Ready_Data> vRoom_Player_Data;

	int ChoiceRoomNum;
	int CurRoom;

	Room_Ready_Data myPlayerData;

};

