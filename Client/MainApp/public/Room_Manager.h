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

private:
	static	Room_Manager* m_pInstance;

	std::vector<Room_Data> vRoom_List;
};

