#pragma once
#include "Room.h"

class Room_Manager
{
private:
	Room_Manager();
	~Room_Manager();

public:

	void		Initialize();
	int			Update(float DeltaTime);
	void		Late_Update(void);
	void		Release(void);

public:

	void ActiveRoom(uint32 roomID);
	void DeActiveRoom(uint32 roomID);

	void ShowRoomDataList();
	void ShowRoomData(uint32 RoomID);

	void Process_Objectdata(RECV_Data input, int ID, int PlayerID);



	// ★ 예전에는 vRooms 가 비어있지 않기만 하면 그대로 vRooms[roomID] 를 돌려줬다.
	//   로비에 있는 플레이어의 RoomNum 은 ROBBY(9999) 라, 로비 상태에서
	//   패킷이 하나만 와도 10칸짜리 벡터를 9999번으로 읽었다.
	Room* Get_Room(uint32 roomID) {
		if (roomID >= vRooms.size())
			return nullptr;
		return vRooms[roomID];
	}

	void		DeleteRoom(uint32 roomID);



public:

	int  Client_CreateRoom(PlayerRef player);
	int  Client_EnterRoom(uint32 RoomID, PlayerRef player);
	bool Client_LeaveRoom(uint32 RoomID, PlayerRef player);


	bool Client_ChangeINFO(uint32 ROOMID, uint64 PlayerID, Room_Ready_Data data);
	bool Ready_Player(uint32 RoomID, uint64 PlayerID);
	void Client_LOADING_FINISH(uint32 ROOMID);

	void BroadCast_LobbyState(uint32 roomID);
	void BroadCast_Game_Start(uint32 roomID);



	void SetTankByRoomIndex(int RoomID,int64 pID,const Matrix4x4& mat, const float& PosinAngle, const float& PotapAngl);
	bool Check_StartGame(uint32 RoomID);

	std::vector<Room_Data> Client_ShowRoom();


public:

	int GetCurPlayer(int RoomID) {
		return vRooms[RoomID]->GetRoomPlayerCnt();

	}

private:

	std::vector<Room*>	vRooms;

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
private:
	static	Room_Manager* m_pInstance;
	FRWLock	m_lock;

};

