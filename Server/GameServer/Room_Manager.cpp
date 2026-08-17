#include "pch.h"
#include "Room_Manager.h"
#include "Player.h"     // 입장 시 RoomNum 을 락 안에서 채운다


Room_Manager* Room_Manager::m_pInstance = nullptr;

Room_Manager::Room_Manager()
{
    vRooms.resize(10);

    for (int i = 0; i < 10; ++i)
    {
        vRooms[i] = new Room();
        vRooms[i]->SetRoomID(i);

    }
}

Room_Manager::~Room_Manager()
{
    Release();
}

void Room_Manager::Initialize()
{
    for (auto& room : vRooms)
    {
        if (room->isActive)
            room->Initialize();
    }

}

// ----------------------------------------------------------------
//  게임 루프 스레드가 부른다.
// ----------------------------------------------------------------
int Room_Manager::Update(float /*DeltaTime*/)
{
    for (auto& room : vRooms)
    {
        if (room && room->isActive)
            room->PushTickJob();
    }

    return 0;
}

/*  Room::TickJob 이 Update 바로 뒤에 부른다.  */
void Room_Manager::Late_Update(void)
{
}

void Room_Manager::Release(void)
{

    for (auto& room : vRooms)
    {
        if (room)
        {
            room->Release();
            delete room;
            room = nullptr;
        }
    }
    vRooms.clear();
}

void Room_Manager::ActiveRoom(uint32 roomID)
{
    WRITE_LOCK(m_lock);
    if (roomID >= vRooms.size())
        return;

    if (!vRooms[roomID]->GetRoomActivate())
    {
        vRooms[roomID]->RoomActivate();
    }
}

void Room_Manager::DeActiveRoom(uint32 roomID)
{
    if (roomID >= vRooms.size())
        return;

    Room* room = vRooms[roomID];
    if (room == nullptr)
        return;

    /*  ResetRoom 은 오브젝트를 delete 하므로 반드시 방 잡 안에서 */
    room->RoomDeActivate();
    room->PushJob([room]() { room->ResetRoom(); });
}

void Room_Manager::ShowRoomDataList()
{
    READ_LOCK(m_lock);
    for (auto& room : vRooms) {
        cout << "ROOM(" << room->GetRoomID() << ")" << "\t";
        cout << "( " << room->GetRoomPlayerCnt() << " / " << room->GetRoomMaxPlayerCnt() << " )" << "\t";
        if (room->GetRoomActivate())
            cout << "활성화";
        else
            cout << "비활성화";
        cout << endl;
    }

}

void Room_Manager::ShowRoomData(uint32 RoomID)
{
    if (RoomID >= vRooms.size())
    {
        SERVER_LOG("존재하지 않는 RoomID " << RoomID);
        return;
    }

    Room* room = vRooms[RoomID];
    if (room == nullptr || !room->GetRoomActivate())
        return;

    room->PushJob([room]() { room->ShowRoomData(); });
}

void Room_Manager::DeleteRoom(uint32 roomID)
{

    if (roomID >= vRooms.size())
        return;

    if (vRooms[roomID])
    {
        vRooms[roomID]->Release();
        delete vRooms[roomID];
        vRooms[roomID] = nullptr;
    }
}

int Room_Manager::Client_CreateRoom(PlayerRef player)
{
    if (player == nullptr)
        return ROOM_CREATE_ERROR;

    Room* pTarget = nullptr;
    {
        WRITE_LOCK(m_lock);     // 두 명이 동시에 만들면 같은 방을 잡을 수 있다

        for (auto& room : vRooms)
        {
            if (room && !room->GetRoomActivate())
            {
                room->RoomActivate();
                room->TakePlayerSlot();

                /*  RoomNum 을 락 안에서 채운다. 밖에서 채우면 그 사이에
                    이 플레이어의 패킷이 들어와도 아직 로비로 보여 버려진다. */
                player->RoomNum = room->GetRoomID();
                pTarget = room;
                break;
            }
        }
    }

    if (pTarget == nullptr)
        return ROOM_CREATE_ERROR;   // 빈 방 없음

    Push_Accept_Player(pTarget, player);
    return pTarget->GetRoomID();
}

int Room_Manager::Client_EnterRoom(uint32 RoomID, PlayerRef player)
{
    if (player == nullptr || RoomID >= vRooms.size())
        return ROOM_ENTER_ERROR;

    Room* pTarget = nullptr;
    {
        WRITE_LOCK(m_lock);

        Room* room = vRooms[RoomID];
        if (!room || !room->GetRoomActivate())
            return ROOM_ENTER_ERROR;

        /*  ★ 검사와 예약이 같은 락 안에 있어야 한다 */
        if (room->isFull())
        {
            SERVER_LOG("Room " << RoomID << " 정원 초과 - 입장 거부");
            return ROOM_ENTER_ERROR;
        }

        room->TakePlayerSlot();
        player->RoomNum = RoomID;
        pTarget = room;
    }

    Push_Accept_Player(pTarget, player);
    SERVER_LOG("Room " << RoomID << " 입장");
    return RoomID;
}

// 예약이 끝난 방에 실제 등록을 맡긴다.
// 잡이 실행될 때까지 Player 가 살아 있다.
void Room_Manager::Push_Accept_Player(Room* pRoom, PlayerRef player)
{
    pRoom->PushJob([pRoom, player]()
        {
            pRoom->Accept_Player(player);
            pRoom->BroadCast_LobbyInfo();
        });
}

void Room_Manager::Client_LeaveRoom(uint32 ID, PlayerRef player)
{
    if (player == nullptr)
        return;

    if (ID >= vRooms.size())
    {
        // 방 번호가 이상하다. 이 Player 를 잡고 있는 방이 없으므로 여기서 끊는다.
        player->OwenerSession.reset();
        return;
    }

    Room* room = vRooms[ID];
    if (!room)
    {
        // 방이 없으면 이 Player 를 참조하는 방 잡도 없다. 여기서 끊어도 안전하다.
        player->OwenerSession.reset();
        return;
    }

    bool bNowEmpty = false;
    {
        WRITE_LOCK(m_lock);

        if (room->GetRoomActivate())
        {
            /*  자리는 즉시 반납한다      */
            room->ReturnPlayerSlot();

            if (room->GetRoomPlayerCnt() == 0)
            {
                room->RoomDeActivate();
                bNowEmpty = true;
            }
        }
    }

    /*  방이 이미 비활성이어도 잡은 반드시 넣는다. */
    room->PushJob([room, player, bNowEmpty]()
        {
            room->Leave_Player(player);     // 명단 제거 + 순환 참조 해제

            if (bNowEmpty)
                room->ResetRoom();
            else if (room->GetRoomActivate())
                room->BroadCast_LobbyInfo();    // 남은 사람들에게 갱신된 명단
        });
}

void Room_Manager::Client_ChangeINFO(uint32 ROOMID, uint64 PlayerID, Room_Ready_Data data)
{
    Room* room = Get_Room(ROOMID);
    if (!room || !room->GetRoomActivate())
        return;

    /*  성공 여부를 밖으로 돌려줄 수 없으므로(비동기), 자리 충돌 검사와
        브로드캐스트를 한 잡 안에서 같이 한다.  */
    room->PushJob([room, PlayerID, data]()
        {
            if (room->Change_Player_Info(PlayerID, data))
                room->BroadCast_LobbyInfo();
        });
}

void Room_Manager::Ready_Player(uint32 RoomID, uint64 PlayerID)
{
    Room* room = Get_Room(RoomID);
    if (!room || !room->GetRoomActivate())
        return;

    room->PushJob([room, PlayerID]()
        {
            if (room->Ready_Player(PlayerID))
                room->BroadCast_LobbyInfo();
        });
}

void Room_Manager::Client_LOADING_FINISH(uint32 ROOMID)
{
    Room* room = Get_Room(ROOMID);
    if (!room || !room->GetRoomActivate())
        return;

    room->PushJob([room]() { room->Clinet_Loading_Finish(); });
}

void Room_Manager::BroadCast_LobbyState(uint32 roomID)
{
    Room* room = Get_Room(roomID);
    if (!room || !room->GetRoomActivate())
        return;

    room->PushJob([room]() { room->BroadCast_LobbyInfo(); });
}

void Room_Manager::Try_Start_Game(uint32 roomID)
{
    Room* room = Get_Room(roomID);
    if (!room || !room->GetRoomActivate())
        return;

    room->PushJob([room]()
        {
            if (room->CanStartGame())
                room->Broadcast_GameStart();
        });
}



std::vector<Room_Data> Room_Manager::Client_ShowRoom()
{

    std::vector<Room_Data> vRoom_Data;

    for (auto& room : vRooms) {

        Room_Data temp;
        temp.MaxPlayer = room->GetRoomMaxPlayerCnt();
        temp.CurPlayer = room->GetRoomPlayerCnt();
        temp.RoomID = room->GetRoomID();
        temp.isActive = room->GetRoomActivate();
        vRoom_Data.push_back(temp);

    }


    return vRoom_Data;
}

void Room_Manager::SetTankByRoomIndex(int RoomID, int64 pID, const Matrix4x4& mat, const float& PosinAngle, const float& PotapAngl)
{
    if (RoomID < 0 || static_cast<size_t>(RoomID) >= vRooms.size())
        return;

    Room* room = vRooms[RoomID];
    if (!room || !room->GetRoomActivate())
        return;

    // 탱크는 방 오브젝트다. 참조 인자를 값으로 복사해 잡에 넘긴다.
    room->PushJob([room, pID, mat, PosinAngle, PotapAngl]()
        {
            room->SetTankState(pID, mat, PosinAngle, PotapAngl);
        });
}


void Room_Manager::Process_Objectdata(RECV_Data input, int RoomID, int PlayerID)
{

    if (RoomID >= vRooms.size())
        return;

    if (!vRooms[RoomID]->GetRoomActivate())
    {
        switch (input) {
        case DATA_TANK_MOVE:

            break;

        case DATA_TANK_SHOT:

            break;

        case DATA_TREE_DELETE:

            break;

        default:
            break;
        }

    }

}