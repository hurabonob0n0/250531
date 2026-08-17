#pragma once
#include "Protocol.h"
#include "Define.h"
#include "SendBuffer.h"
#include <memory>
#include <vector>

using SessionRef = std::shared_ptr<class CSession>;

class CPacket_Handler
{
public:
    static void Handle(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);

    // ---- 수신 ----
    static void Handle_C_LOGIN(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_KEYINPUT(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);

    // 방
    static void Handle_C_SHOW_ROOM(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_CREATE_ROOM(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_JOIN_ROOM(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_EXIT_ROOM(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_CHANGE_INFO(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_READY(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_GAMESTART(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);

    // 인게임
    static void Handle_C_LOADING_FINISH(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_MOVEMENT(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_POS_MOVE(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_POSIN_MOVE(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_SHOT(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_TANK_RESPAWN(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_DRONE_MOVE(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_AIRDROP(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_TANKSOUND(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);
    static void Handle_C_ADD_PING(SessionRef pSession, uint8_t* pBuffer, int32_t nSize);

    // ---- 송신 버퍼 생성 ----
    static SendBufferRef Make_S_SUCCESS_LOGIN(uint16 id);

    // 로비
    static SendBufferRef Make_S_ROOM_DATA();
    static SendBufferRef Make_S_ROOM_PLAYER_STATES(const std::vector<Room_Ready_Data>& dataList);
    static SendBufferRef Make_S_GAME_START(uint8 dummy);
    static SendBufferRef Make_S_ALL_PLAYER_LOADING_FINISH(uint8 dummy);

    // 인게임
    static SendBufferRef Make_S_ALL_TANK_STATE(std::vector<Tank_INFO>& tanks);
    static SendBufferRef Make_S_ALL_DRONE_STATE(std::vector<Drone_INFO>& drones);
    static SendBufferRef Make_S_WEAPON_HIT(float x, float y, float z);
    static SendBufferRef Make_S_TANK_HIT(uint8 id);
    static SendBufferRef Make_S_TANK_DAMAGED(uint8 id);
    static SendBufferRef Make_S_TANK_DEAD(uint8 id);
    static SendBufferRef Make_S_TANK_KILL(uint8 id);
    static SendBufferRef Make_S_RespawnTank(uint8 tankIndex);
    static SendBufferRef Make_S_GAME_WIN(uint8 dummy);
    static SendBufferRef Make_S_GAME_LOSE(uint8 dummy);
    static SendBufferRef Make_S_CAPTURE(uint8 blue, uint8 red);
    static SendBufferRef Make_S_BULLETADD(uint8 tankIndex, float dirX, float dirY, float dirZ,
                                          float posX, float posY, float posZ);
    static SendBufferRef Make_S_AIRDROP(uint8 areaIndex);
    static SendBufferRef Make_S_SOUND(uint8 tankIndex, float engVol, float engPit,
                                      float trkVol, float trkPit);
    static SendBufferRef Make_S_PINGPOS(uint8 tankIndex, float x, float y, float z);
};
