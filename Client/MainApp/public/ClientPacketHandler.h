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
	C_LOGIN = 1001,
	C_KEYINPUT = 1002,
	C_MOVEMENT = 1003,
	C_SHOT = 1004
};


class ClientPacketHandler
{
public:
	static void HandlePacket(BYTE* buffer, int32 len);
	static void Handle_S_TEST(BYTE* buffer, int32 len);
	static void Handle_S_SUCCESS_LOGIN(BYTE* buffer, int32 len);
	static void Handle_S_GAME_START(BYTE* buffer, int32 len);
	static void Handle_S_PLAYER_MOVE(BYTE* buffer, int32 len);
	static void Handle_S_SUCCESS_ENTER_ROOM(BYTE* buffer, int32 len);

	static SendBufferRef Make_C_LOGIN(uint64 id);
	static SendBufferRef Make_C_KEYINPUT(uint8 key);
	static SendBufferRef Make_C_MOVE(_float4x4& worldMatrix, float potapRotation, float posinRotation);

	//static SendBufferRef Make_C_SHOT(uint, uint16 attack);

	class CGameInstance* m_GameInstance = {};

};
