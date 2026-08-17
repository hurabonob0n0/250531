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

	Room* Get_Room(uint32 roomID) {
		if (roomID >= vRooms.size())
			return nullptr;
		return vRooms[roomID];
	}

	void		DeleteRoom(uint32 roomID);

private:
	// 예약이 끝난 방에 실제 명단 등록 잡을 넘긴다(Create/Enter 공통).
	void Push_Accept_Player(Room* pRoom, PlayerRef player);

public:



public:
	// ================================================================
	//  로비 조작
	//  아래 함수들은 방 상태를 직접 바꾸지 않고 잡 큐에 밀어 넣은 즉시 돌아온다.
	// ================================================================

	// 성공하면 방 번호, 실패하면 ROOM_CREATE_ERROR / ROOM_ENTER_ERROR.
	// 성공 시 player->RoomNum 도 이 안에서 채운다.
	int  Client_CreateRoom(PlayerRef player);
	int  Client_EnterRoom(uint32 RoomID, PlayerRef player);

	// 자리를 즉시 반납하고, 명단 제거와 로비 통보는 방 잡에서 한다.
	void Client_LeaveRoom(uint32 RoomID, PlayerRef player);

	void Client_ChangeINFO(uint32 ROOMID, uint64 PlayerID, Room_Ready_Data data);
	void Ready_Player(uint32 RoomID, uint64 PlayerID);
	void Client_LOADING_FINISH(uint32 ROOMID);

	void BroadCast_LobbyState(uint32 roomID);

	// 전원 Ready 인지 확인하는 것까지 잡 안에서 한다(예전 Check_StartGame + BroadCast).
	void Try_Start_Game(uint32 roomID);

	void SetTankByRoomIndex(int RoomID,int64 pID,const Matrix4x4& mat, const float& PosinAngle, const float& PotapAngl);

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

