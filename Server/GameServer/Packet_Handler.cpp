#include "pch.h"
#include "Packet_Handler.h"
#include "Session.h"
#include "Player.h"
#include "Room.h"
#include "Room_Manager.h"

// 한 패킷이 쓸 수 있는 최대 크기.
static constexpr int32_t PACKET_BUF_SIZE = 4096;

// 로그인할 때마다 하나씩 늘어나는 플레이어 번호.
static Atomic<uint64> g_ClientID = 0;

// ----------------------------------------------------------------
//  세션 -> 플레이어 -> 방을 한 번에 꺼낸다.
// ----------------------------------------------------------------
static Room* GetRoomOf(const PlayerRef& pPlayer)
{
    if (!pPlayer) return nullptr;
    return Room_Manager::Get_Instance()->Get_Room(static_cast<uint32>(pPlayer->RoomNum));
}

void CPacket_Handler::Handle(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    FPacketReader br(pBuffer, nSize);

    PacketHeader header{};
    if (!br.Peek(&header))
        return;

    switch (header.id)
    {
    case C_LOGIN:           Handle_C_LOGIN(pSession, pBuffer, nSize);           break;
    case C_KEYINPUT:        Handle_C_KEYINPUT(pSession, pBuffer, nSize);        break;
    case C_MOVEMENT:        Handle_C_MOVEMENT(pSession, pBuffer, nSize);        break;
    case C_SHOT:            Handle_C_SHOT(pSession, pBuffer, nSize);            break;
    case C_RESPAWN_TANK:    Handle_C_TANK_RESPAWN(pSession, pBuffer, nSize);    break;
    case C_SHOW_ROOM:       Handle_C_SHOW_ROOM(pSession, pBuffer, nSize);       break;
    case C_CREATE_ROOM:     Handle_C_CREATE_ROOM(pSession, pBuffer, nSize);     break;
    case C_JOIN_ROOM:       Handle_C_JOIN_ROOM(pSession, pBuffer, nSize);       break;
    case C_CHANGE_INFO:     Handle_C_CHANGE_INFO(pSession, pBuffer, nSize);     break;
    case C_EXIT_ROOM:       Handle_C_EXIT_ROOM(pSession, pBuffer, nSize);       break;
    case C_READY:           Handle_C_READY(pSession, pBuffer, nSize);           break;
    case C_START:           Handle_C_GAMESTART(pSession, pBuffer, nSize);       break;
    case C_FINISH_LOADING:  Handle_C_LOADING_FINISH(pSession, pBuffer, nSize);  break;
    case C_MYPOS:           Handle_C_POS_MOVE(pSession, pBuffer, nSize);        break;
    case C_MYPOSIN:         Handle_C_POSIN_MOVE(pSession, pBuffer, nSize);      break;
    case C_MYDRONEMOVE:     Handle_C_DRONE_MOVE(pSession, pBuffer, nSize);      break;
    case C_AIRDROP:         Handle_C_AIRDROP(pSession, pBuffer, nSize);         break;
    case C_TANK_SOUND:      Handle_C_TANKSOUND(pSession, pBuffer, nSize);       break;
    case C_ADD_PING:        Handle_C_ADD_PING(pSession, pBuffer, nSize);        break;
    default:                                                                    break;
    }
}

// ================================================================
//  수신
// ================================================================

void CPacket_Handler::Handle_C_LOGIN(SessionRef pSession, uint8_t* /*pBuffer*/, int32_t /*nSize*/)
{
    // 이미 로그인한 세션이 다시 보내면 무시한다(플레이어가 두 개 생기는 것을 막음).
    if (pSession->GetPlayer())
        return;

    const uint64 nNewClientID = g_ClientID.fetch_add(1);

    PlayerRef pPlayer      = std::make_shared<Player>();
    pPlayer->playerID      = nNewClientID;
    pPlayer->RoomNum       = ROBBY;
    pPlayer->OwenerSession = pSession;

    pSession->SetPlayer(pPlayer);

    pSession->Send(Make_S_SUCCESS_LOGIN(static_cast<uint16>(nNewClientID)));
}

void CPacket_Handler::Handle_C_KEYINPUT(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    if (!pPlayer) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    uint8         key = 0;

    br >> header >> key;
}

void CPacket_Handler::Handle_C_MOVEMENT(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    Room*     pRoom   = GetRoomOf(pPlayer);
    if (!pRoom) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    br >> header;

    uint8 tankIndex = 0;
    br >> tankIndex;

    Vec3 pos;
    Quat rot;
    br >> pos.X >> pos.Y >> pos.Z
       >> rot.X >> rot.Y >> rot.Z >> rot.W;

    float potapRotation = 0.f;
    float posinRotation = 0.f;
    br >> potapRotation >> posinRotation;

    int8 wheelSag[TANK_WHEEL_COUNT] = {};
    for (int i = 0; i < TANK_WHEEL_COUNT; ++i)
        br >> wheelSag[i];

    /* 조각난 패킷이면 값이 안 채워져 있다. 그대로 넣으면 탱크가 원점으로 날아간다. */
    if (br.IsUnderflow())
        return;

    pRoom->SetTankStateQuat(tankIndex, pos, rot, potapRotation, posinRotation, wheelSag);
}

void CPacket_Handler::Handle_C_POS_MOVE(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    Room*     pRoom   = GetRoomOf(pPlayer);
    if (!pRoom) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    br >> header;

    uint8 tankIndex = 0;
    br >> tankIndex;

    Vec3 pos;
    Quat rot;
    br >> pos.X >> pos.Y >> pos.Z
       >> rot.X >> rot.Y >> rot.Z >> rot.W;

    int8 wheelSag[TANK_WHEEL_COUNT] = {};
    for (int i = 0; i < TANK_WHEEL_COUNT; ++i)
        br >> wheelSag[i];

    if (br.IsUnderflow())
        return;

    pRoom->SetTankPosQuat(tankIndex, pos, rot, wheelSag);
}

void CPacket_Handler::Handle_C_POSIN_MOVE(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    Room*     pRoom   = GetRoomOf(pPlayer);
    if (!pRoom) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    br >> header;

    uint8 tankIndex     = 0;
    float potapRotation = 0.f;
    float posinRotation = 0.f;
    br >> tankIndex >> potapRotation >> posinRotation;

    pRoom->SetTankPosin(tankIndex, potapRotation, posinRotation);
}

void CPacket_Handler::Handle_C_SHOT(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    Room*     pRoom   = GetRoomOf(pPlayer);
    if (!pRoom) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    br >> header;

    uint8 tankIndex = 0;
    Vec3  initPos;
    Vec3  normalizedDir;
    br >> tankIndex
       >> initPos.X >> initPos.Y >> initPos.Z
       >> normalizedDir.X >> normalizedDir.Y >> normalizedDir.Z;

    pRoom->CreateBullet(static_cast<int8>(pPlayer->playerID), tankIndex,
                        WEAPON_NPOTAN, normalizedDir, initPos);
}

void CPacket_Handler::Handle_C_TANK_RESPAWN(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    Room*     pRoom   = GetRoomOf(pPlayer);
    if (!pRoom) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    br >> header;

    uint8 tankIndex = 0;
    br >> tankIndex;

    Matrix4x4 mat;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            br >> mat.m[i][j];

    float potapRotation = 0.f;
    float posinRotation = 0.f;
    br >> potapRotation >> posinRotation;

    pRoom->SetTankRespawn(tankIndex, mat, potapRotation, posinRotation);
    pRoom->Send_RespawnPacket(tankIndex);
}

void CPacket_Handler::Handle_C_DRONE_MOVE(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    Room*     pRoom   = GetRoomOf(pPlayer);
    if (!pRoom) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    br >> header;

    uint8 droneIndex = 0;
    float posX = 0.f, posY = 0.f, posZ = 0.f;
    float yaw = 0.f, roll = 0.f, pitch = 0.f;

    br >> droneIndex >> posX >> posY >> posZ >> yaw >> roll >> pitch;

    pRoom->SetDroneState(droneIndex, Vec3(posX, posY, posZ), yaw, roll, pitch);
}

void CPacket_Handler::Handle_C_AIRDROP(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    Room*     pRoom   = GetRoomOf(pPlayer);
    if (!pRoom) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    br >> header;

    uint8 tankIndex = 0;
    uint8 areaIndex = 0;
    br >> tankIndex >> areaIndex;

    pRoom->CreateBomb(static_cast<uint8>(pPlayer->playerID), tankIndex, areaIndex);
}

void CPacket_Handler::Handle_C_TANKSOUND(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    Room*     pRoom   = GetRoomOf(pPlayer);
    if (!pRoom) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    br >> header;

    uint8 tankIndex = 0;
    float engVol = 0.f, engPit = 0.f, trkVol = 0.f, trkPit = 0.f;
    br >> tankIndex >> engVol >> engPit >> trkVol >> trkPit;

    pRoom->Send_SoundData(tankIndex, engVol, engPit, trkVol, trkPit);
}

void CPacket_Handler::Handle_C_ADD_PING(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    Room*     pRoom   = GetRoomOf(pPlayer);
    if (!pRoom) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    br >> header;

    uint8 tankIndex = 0;
    float x = 0.f, y = 0.f, z = 0.f;
    br >> tankIndex >> x >> y >> z;

    pRoom->Send_PingData(tankIndex, x, y, z);
}

// ---- 방 ----

void CPacket_Handler::Handle_C_SHOW_ROOM(SessionRef pSession, uint8_t* /*pBuffer*/, int32_t /*nSize*/)
{
    pSession->Send(Make_S_ROOM_DATA());
}

void CPacket_Handler::Handle_C_CREATE_ROOM(SessionRef pSession, uint8_t* /*pBuffer*/, int32_t /*nSize*/)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    if (!pPlayer) return;

    const int nRoomID = Room_Manager::Get_Instance()->Client_CreateRoom(pPlayer);
    if (nRoomID == ROOM_CREATE_ERROR)
        return;     // 빈 방이 없다

    pPlayer->RoomNum = nRoomID;
    Room_Manager::Get_Instance()->BroadCast_LobbyState(nRoomID);
}

void CPacket_Handler::Handle_C_JOIN_ROOM(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    if (!pPlayer) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    uint32        nRoomID = 0;
    br >> header >> nRoomID;

    const int nResult = Room_Manager::Get_Instance()->Client_EnterRoom(nRoomID, pPlayer);
    if (nResult == ROOM_ENTER_ERROR)
        return;     // 입장 실패면 RoomNum 을 바꾸면 안 된다

    pPlayer->RoomNum = nResult;
    Room_Manager::Get_Instance()->BroadCast_LobbyState(nResult);
}

void CPacket_Handler::Handle_C_EXIT_ROOM(SessionRef pSession, uint8_t* /*pBuffer*/, int32_t /*nSize*/)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    if (!pPlayer) return;
    if (pPlayer->RoomNum == ROBBY) return;

    const uint32 nRoomID = static_cast<uint32>(pPlayer->RoomNum);

    Room_Manager::Get_Instance()->Client_LeaveRoom(nRoomID, pPlayer);
    Room_Manager::Get_Instance()->BroadCast_LobbyState(nRoomID);
    pPlayer->RoomNum = ROBBY;
}

void CPacket_Handler::Handle_C_CHANGE_INFO(SessionRef pSession, uint8_t* pBuffer, int32_t nSize)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    if (!pPlayer || pPlayer->RoomNum == ROBBY) return;

    FPacketReader br(pBuffer, nSize);
    PacketHeader  header{};
    br >> header;

    Room_Ready_Data data{};
    br >> data.PlayerID >> data.Position >> data.Team >> data.IsReady;

    const uint32 nRoomID = static_cast<uint32>(pPlayer->RoomNum);

    if (Room_Manager::Get_Instance()->Client_ChangeINFO(nRoomID, pPlayer->playerID, data))
        Room_Manager::Get_Instance()->BroadCast_LobbyState(nRoomID);
}

void CPacket_Handler::Handle_C_READY(SessionRef pSession, uint8_t* /*pBuffer*/, int32_t /*nSize*/)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    if (!pPlayer || pPlayer->RoomNum == ROBBY) return;

    const uint32 nRoomID = static_cast<uint32>(pPlayer->RoomNum);

    if (Room_Manager::Get_Instance()->Ready_Player(nRoomID, pPlayer->playerID))
        Room_Manager::Get_Instance()->BroadCast_LobbyState(nRoomID);
}

void CPacket_Handler::Handle_C_GAMESTART(SessionRef pSession, uint8_t* /*pBuffer*/, int32_t /*nSize*/)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    if (!pPlayer || pPlayer->RoomNum == ROBBY) return;

    const uint32 nRoomID = static_cast<uint32>(pPlayer->RoomNum);

    if (Room_Manager::Get_Instance()->Check_StartGame(nRoomID))
        Room_Manager::Get_Instance()->BroadCast_Game_Start(nRoomID);
}

void CPacket_Handler::Handle_C_LOADING_FINISH(SessionRef pSession, uint8_t* /*pBuffer*/, int32_t /*nSize*/)
{
    PlayerRef pPlayer = pSession->GetPlayer();
    if (!pPlayer || pPlayer->RoomNum == ROBBY) return;

    Room_Manager::Get_Instance()->Client_LOADING_FINISH(static_cast<uint32>(pPlayer->RoomNum));
}

// ================================================================
//  송신 버퍼 생성
// ================================================================

// 헤더 자리 잡기 + 닫기를 매번 반복하지 않도록 묶은 헬퍼.
namespace
{
    struct FPacketScope
    {
        SendBufferRef buffer;
        FPacketWriter bw;
        PacketHeader* header = nullptr;

        FPacketScope()
            : buffer(MakeSendBuffer(PACKET_BUF_SIZE))
        {
            bw     = FPacketWriter(buffer->Buffer(), buffer->AllocSize());
            header = bw.Reserve<PacketHeader>();
        }

        SendBufferRef Close(uint16 id)
        {
            if (header == nullptr)
                return nullptr;

            header->size = static_cast<uint16>(bw.WriteSize());
            header->id   = id;
            buffer->Close(bw.WriteSize());
            return buffer;
        }
    };
}

SendBufferRef CPacket_Handler::Make_S_SUCCESS_LOGIN(uint16 id)
{
    FPacketScope p;
    p.bw << id;
    return p.Close(S_SUCCESS_LOGIN);
}

SendBufferRef CPacket_Handler::Make_S_ROOM_DATA()
{
    FPacketScope p;

    std::vector<Room_Data> rooms = Room_Manager::Get_Instance()->Client_ShowRoom();

    p.bw << static_cast<uint32>(rooms.size());
    for (const Room_Data& data : rooms)
        p.bw << data;

    return p.Close(S_ROOM_DATA);
}

SendBufferRef CPacket_Handler::Make_S_ROOM_PLAYER_STATES(const std::vector<Room_Ready_Data>& dataList)
{
    FPacketScope p;

    p.bw << static_cast<uint16>(dataList.size());
    for (const Room_Ready_Data& data : dataList)
        p.bw << data.PlayerID << data.Position << data.Team << data.IsReady;

    return p.Close(S_ROOM_PLAYER_STATES);
}

SendBufferRef CPacket_Handler::Make_S_GAME_START(uint8 dummy)
{
    FPacketScope p;
    p.bw << dummy;
    return p.Close(S_GAME_START);
}

SendBufferRef CPacket_Handler::Make_S_ALL_PLAYER_LOADING_FINISH(uint8 dummy)
{
    FPacketScope p;
    p.bw << dummy;
    return p.Close(S_ROOM_ALL_PLAYER_FINISH_LOADING);
}

SendBufferRef CPacket_Handler::Make_S_ALL_TANK_STATE(std::vector<Tank_INFO>& tanks)
{
    FPacketScope p;

    p.bw << static_cast<uint16>(tanks.size());

    uint8 tankIndex = 0;
    for (Tank_INFO& tank : tanks)
    {
        p.bw << tankIndex++;

        p.bw << tank.Pos.X << tank.Pos.Y << tank.Pos.Z;
        p.bw << tank.Rot.X << tank.Rot.Y << tank.Rot.Z << tank.Rot.W;

        p.bw << tank.PotapAngle;
        p.bw << tank.PosinAngle;
        p.bw << tank.TankHP;

        for (int i = 0; i < TANK_WHEEL_COUNT; ++i)
            p.bw << tank.WheelSag[i];
    }

    return p.Close(S_ALL_TANK_STATE);
}

SendBufferRef CPacket_Handler::Make_S_ALL_DRONE_STATE(std::vector<Drone_INFO>& drones)
{
    FPacketScope p;

    p.bw << static_cast<uint16>(drones.size());

    uint8 droneIndex = 0;
    for (Drone_INFO& drone : drones)
    {
        p.bw << droneIndex++;
        p.bw << drone.DroneTransform.X << drone.DroneTransform.Y << drone.DroneTransform.Z;
        p.bw << drone.Yaw << drone.Roll << drone.Pitch;
        p.bw << drone.DroneHP;
    }

    return p.Close(S_ALL_DRONE_STATE);
}

SendBufferRef CPacket_Handler::Make_S_WEAPON_HIT(float x, float y, float z)
{
    FPacketScope p;
    p.bw << x << y << z;
    return p.Close(S_WEAPON_HIT);
}

SendBufferRef CPacket_Handler::Make_S_TANK_HIT(uint8 id)
{
    FPacketScope p;
    p.bw << id;
    return p.Close(S_TANK_HIT);
}

SendBufferRef CPacket_Handler::Make_S_TANK_DAMAGED(uint8 id)
{
    FPacketScope p;
    p.bw << id;
    return p.Close(S_TANK_DAMAGED);
}

SendBufferRef CPacket_Handler::Make_S_TANK_DEAD(uint8 id)
{
    FPacketScope p;
    p.bw << id;
    return p.Close(S_TANK_DEAD);
}

SendBufferRef CPacket_Handler::Make_S_TANK_KILL(uint8 id)
{
    FPacketScope p;
    p.bw << id;
    return p.Close(S_TANK_KILL);
}

SendBufferRef CPacket_Handler::Make_S_RespawnTank(uint8 tankIndex)
{
    FPacketScope p;
    p.bw << tankIndex;
    return p.Close(S_TANK_RESPAWN);
}

SendBufferRef CPacket_Handler::Make_S_GAME_WIN(uint8 dummy)
{
    FPacketScope p;
    p.bw << dummy;
    return p.Close(S_GAME_WIN);
}

SendBufferRef CPacket_Handler::Make_S_GAME_LOSE(uint8 dummy)
{
    FPacketScope p;
    p.bw << dummy;
    return p.Close(S_GAME_LOSE);
}

SendBufferRef CPacket_Handler::Make_S_CAPTURE(uint8 blue, uint8 red)
{
    FPacketScope p;
    p.bw << blue << red;
    return p.Close(S_CAPTURE);
}

SendBufferRef CPacket_Handler::Make_S_BULLETADD(uint8 tankIndex,
                                                float dirX, float dirY, float dirZ,
                                                float posX, float posY, float posZ)
{
    FPacketScope p;
    p.bw << tankIndex << dirX << dirY << dirZ << posX << posY << posZ;
    return p.Close(S_BULLET_ADD);
}

SendBufferRef CPacket_Handler::Make_S_AIRDROP(uint8 areaIndex)
{
    FPacketScope p;
    p.bw << areaIndex;
    return p.Close(S_AIRDROP_INDEX);
}

SendBufferRef CPacket_Handler::Make_S_SOUND(uint8 tankIndex,
                                            float engVol, float engPit,
                                            float trkVol, float trkPit)
{
    FPacketScope p;
    p.bw << tankIndex << engVol << engPit << trkVol << trkPit;
    return p.Close(S_TANK_SOUND);
}

SendBufferRef CPacket_Handler::Make_S_PINGPOS(uint8 tankIndex, float x, float y, float z)
{
    FPacketScope p;
    p.bw << tankIndex << x << y << z;
    return p.Close(S_ADD_PING);
}
