#pragma once
#include "Session.h"
#include "RenderObject.h"

BEGIN(Engine)
class CGameInstance;
END

enum
{
	S_TEST = 1,//for Dummy
	S_SUCCESS_LOGIN = 2,
	S_SUCCESS_ENTER_ROOM = 3,
	S_GAME_START = 4,
	S_ALL_TANK_STATE = 5,
	S_WEAPON_HIT = 6,
	S_ROOM_DATA = 7,
	S_ROOM_ENTER = 8,
	S_ROOM_PLAYER_STATES = 9,
	S_ROOM_ALL_PLAYER_FINISH_LOADING = 10,
	S_TANK_HIT = 11,
	S_TANK_DAMAGED = 12,
	S_TANK_DEAD = 13,
	S_TANK_KILL = 14,
	S_GAME_WIN = 15,
	S_GAME_LOSE = 16,
	S_CAPTURE = 17,
	S_ALL_DRONE_STATE = 18,
	S_BULLET_ADD = 19,

	C_LOGIN = 1001,
	C_FINISH_LOADING = 1002,
	C_KEYINPUT = 1003,
	C_MOVEMENT = 1004,
	C_SHOT = 1005,
	C_SHOW_ROOM = 1006,
	C_CREATE_ROOM = 1007,
	C_JOIN_ROOM = 1008,
	C_EXIT_ROOM = 1009,
	C_CHANGE_INFO = 1010,
	C_READY = 1011,
	C_START = 1012,
	C_RESPAWN_TANK = 1013,
	C_MYPOS = 1014,
	C_MYPOSIN = 1015,
	C_MYDRONEMOVE = 1016,
	C_AIRDROP = 1017

};





class ClientPacketHandler
{
public:

	static void HandlePacket(BYTE* buffer, int32 len);
	static void Handle_S_TEST(BYTE* buffer, int32 len);
	
	
	
	static void Handle_S_SUCCESS_LOGIN(BYTE* buffer, int32 len);
	
	static void Handle_S_GET_ROOMDATA(BYTE* buffer, int32 len);
	static void Handle_S_ROOM_ENTER(BYTE* buffer, int32 len);
	static void Handle_S_ROOM_PLAYER_STATES(BYTE* buffer, int32 len);
	static void Handle_S_GAME_START(BYTE* buffer, int32 len);
	
	
	static void Handle_S_ROOM_ALL_PLAYER_FINISH_LOADING(BYTE* buffer, int32 len);
	static void Handle_S_ALL_TANK_STATE(BYTE* buffer, int32 len);
	static void Handle_S_ALL_DRONE_STATE(BYTE* buffer, int32 len);
	static void Handle_S_SUCCESS_ENTER_ROOM(BYTE* buffer, int32 len);
	static void Handle_S_WEAPON_HIT(BYTE* buffer, int32 len);
	static void Handle_S_BULLET_ADD(BYTE* buffer, int32 len);


	static void Handle_S_HIT_TANK(BYTE* buffer, int32 len);
	static void Handle_S_DAMAGED_TANK(BYTE* buffer, int32 len);
	static void Handle_S_DEAD_TANK(BYTE* buffer, int32 len);
	static void Handle_S_KILL_TANK(BYTE* buffer, int32 len);
	static void Handle_S_GAME_WIN(BYTE* buffer, int32 len);
	static void Handle_S_GAME_LOSE(BYTE* buffer, int32 len);
	static void Handle_S_CAPTURE(BYTE* buffer, int32 len);
	
	
	
	//static void Handle_S_PLAYER_MOVE(BYTE* buffer, int32 len);



	static SendBufferRef Make_C_MOVE(_float4x4& worldMatrix, float potapRotation, float posinRotation);
	static SendBufferRef Make_C_SHOT(float PosX, float PosY, float PosZ, float nDirX, float nDirY, float nDirZ);

	static SendBufferRef Make_C_DRONE_MOVE(_float4x4& worldMatrix);

	static SendBufferRef Make_C_LOGIN(uint64 id);
	static SendBufferRef Make_C_KEYINPUT(uint8 key);
	static SendBufferRef Make_C_MOVE(float x, float y, float z);
	static SendBufferRef Make_C_SHOWROOM(uint8 Dummy);
	static SendBufferRef Make_C_JOINROOM(uint32 RoomNum);
	static SendBufferRef Make_C_CREATEROOM(uint8 Dummy);
	static SendBufferRef Make_C_EXITROOM(uint8 Dummy);
	static SendBufferRef Make_C_CHANGE_INFO(Room_Ready_Data data);
	static SendBufferRef Make_C_READY(uint8 dummy);
	static SendBufferRef Make_C_START(uint8 dummy);

	static SendBufferRef Make_C_LOADING_FINISH(uint8 dummy);
	static SendBufferRef Make_C_TANK_RESPAWN(_float4x4& worldMatrix, float potapRotation, float posinRotation);

	static SendBufferRef Make_C_TANK_POSINMOVE(float potapRotation, float posinRotation);
	static SendBufferRef Make_C_TANK_POSMOVE(_float4x4& worldMatrix);

	static SendBufferRef Make_C_AIRDROP(uint8 AreaNum);
	//For GamePlay


	USE_LOCK;
	
	//static SendBufferRef Make_C_SHOT(uint, uint16 attack);



	class CGameInstance* m_GameInstance = {};

};