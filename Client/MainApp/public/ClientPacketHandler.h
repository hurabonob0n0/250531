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
	S_PLAYER_MOVE = 5,
	S_WEAPON_HIT = 6,
	S_ROOM_DATA = 7,
	S_ROOM_ENTER = 8,
	S_ROOM_PLAYER_STATES = 9,
	C_LOGIN = 1001,
	C_FINISH_LOADING = 1002,
	C_KEYINPUT = 1003,
	C_MOVEMENT = 1004,
	C_SHOT = 1005,
	C_SHOW_ROOM = 1006,
	C_CREATE_ROOM = 1007,
	C_JOIN_ROOM = 1008,
	C_EXIT_ROOM = 1009,
	C_CHANGE_INFO = 1010
};




class ClientPacketHandler
{
public:

	static void HandlePacket(BYTE* buffer, int32 len);
	static void Handle_S_TEST(BYTE* buffer, int32 len);
	static void Handle_S_SUCCESS_LOGIN(BYTE* buffer, int32 len);
	static void Handle_S_GAME_START(BYTE* buffer, int32 len);
	static void Handle_S_PLAYER_MOVE(BYTE* buffer, int32 len);
	static void Handle_S_GET_ROOMDATA(BYTE* buffer, int32 len);
	static void Handle_S_ROOM_ENTER(BYTE* buffer, int32 len);
	static void Handle_S_ROOM_PLAYER_STATES(BYTE* buffer, int32 len);

	static void Handle_S_SUCCESS_ENTER_ROOM(BYTE* buffer, int32 len);
	static void Handle_S_WEAPON_HIT(BYTE* buffer, int32 len);

	static SendBufferRef Make_C_MOVE(_float4x4& worldMatrix, float potapRotation, float posinRotation);
	static SendBufferRef Make_C_SHOT(float PosX, float PosY, float PosZ, float nDirX, float nDirY, float nDirZ);
	static SendBufferRef Make_C_LOGIN(uint64 id);
	static SendBufferRef Make_C_KEYINPUT(uint8 key);
	static SendBufferRef Make_C_MOVE(float x, float y, float z);
	static SendBufferRef Make_C_SHOWROOM(uint8 Dummy);
	static SendBufferRef Make_C_JOINROOM(uint32 RoomNum);
	static SendBufferRef Make_C_CREATEROOM(uint8 Dummy);
	static SendBufferRef Make_C_EXITROOM(uint8 Dummy);
	static SendBufferRef Make_C_CHANGE_INFO(Room_Ready_Data data);
	USE_LOCK;
	
	//static SendBufferRef Make_C_SHOT(uint, uint16 attack);



	class CGameInstance* m_GameInstance = {};

};