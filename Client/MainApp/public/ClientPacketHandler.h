#pragma once
#include "Protocol.h"
#include "SendBuffer.h"
#include "RenderObject.h"



BEGIN(Engine)
class CGameInstance;
END



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
	static void Handle_S_AIRDROP(BYTE* buffer, int32 len);
	static void Handle_S_RESPAWN(BYTE* buffer, int32 len);
	static void Handle_S_SOUND(BYTE* buffer, int32 len);

	static void Handle_S_ADD_PING(BYTE* buffer, int32 len);


	static SendBufferRef Make_C_MOVE(_float4x4& worldMatrix, float potapRotation, float posinRotation, const int8* pWheelSag);
	static SendBufferRef Make_C_SHOT(float PosX, float PosY, float PosZ, float nDirX, float nDirY, float nDirZ);

	static SendBufferRef Make_C_DRONE_MOVE(float PosX, float PosY, float PosZ, float Yaw, float Roll, float Pitch);

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
	static SendBufferRef Make_C_TANK_POSMOVE(_float4x4& worldMatrix, const int8* pWheelSag);

	static SendBufferRef Make_C_AIRDROP(uint8 AreaNum);
	static SendBufferRef Make_C_SOUND(float EngineVol,float EnginePitch, float TrackVol, float TrackPitch);
	static SendBufferRef Make_C_PING(float X, float Y, float Z);

	//For GamePlay

	class CGameInstance* m_GameInstance = {};

};