#include "Client_pch.h"
#include "Client_Globals.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "ServiceManager.h"
#include "GameInstance.h"


void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case S_TEST:
		Handle_S_TEST(buffer, len);
		break;

	case S_SUCCESS_LOGIN:
		Handle_S_SUCCESS_LOGIN(buffer, len);
		break;

	case S_SUCCESS_ENTER_ROOM:
		Handle_S_SUCCESS_ENTER_ROOM(buffer, len);
		break;

	case S_GAME_START:
		Handle_S_GAME_START(buffer, len);
		break;

	case S_PLAYER_MOVE:
		Handle_S_PLAYER_MOVE(buffer, len);
		break;
	


	default:
		break;
	}
}


struct S_TEST
{
	uint64 id;
	uint32 hp;
	uint16 attack;

};

struct S_Pos {
	float PosX;
	float PosY;
	float PosZ;
};

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	uint64 id;
	uint32 hp;
	uint16 attack;
	br >> id >> hp >> attack;

	cout << "ID: " << id << " HP : " << hp << " ATT : " << attack << endl;
}

void ClientPacketHandler::Handle_S_SUCCESS_LOGIN(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);
	PacketHeader header;

	br >> header;
	uint16 ID;

#pragma region ID값 객체에 넣어주기

	br >> ID;
	ServiceManager::GetInstace().SetMyID(ID);
	g_PlayerID.store(ID);
	g_ServerConnected.store(true);

#pragma endregion
}

void ClientPacketHandler::Handle_S_GAME_START(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	g_GameStart.store(true);

}

void ClientPacketHandler::Handle_S_SUCCESS_ENTER_ROOM(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	uint16 dummy;
	br >> dummy;


}



void ClientPacketHandler::Handle_S_PLAYER_MOVE(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	float PlayersPos[3];
	br >> PlayersPos[0] >> PlayersPos[1] >> PlayersPos[2];

	if (g_PlayerID.load() == 0) {
		CGameInstance::Get_Instance()->Set_Pos_For_Server("Tank", 1, PlayersPos);

	}
	else {

		CGameInstance::Get_Instance()->Set_Pos_For_Server("Tank", 0, PlayersPos);
	}
	//otherPosX = PlayersPos[0];
	//otherPosY = PlayersPos[1];
	//otherPosZ = PlayersPos[2];

#pragma region 객체들 좌표값 넣어주기
	uint16 id = ServiceManager::GetInstace().GetMyID();
	//ex) objectlist.Get_object(objbox,id).set_pos(PlayersPos[id]);

#pragma endregion TODO : LOCK걸고 PlayerID에 맞는 값 넣어주기
}



SendBufferRef ClientPacketHandler::Make_C_LOGIN(uint64 id)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();
	bw << id;

	header->size = bw.WriteSize();
	header->id = C_LOGIN;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_KEYINPUT(uint8 key)
{

	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();
	bw << key;

	header->size = bw.WriteSize();
	header->id = C_KEYINPUT;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_MOVE(_float4x4& worldMatrix, float potapRotation, float posinRotation)
{
	
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();
	
	for (int i = 0; i < 4; ++i)
	{
		bw << worldMatrix.m[i][0];  // row i, col 0
		bw << worldMatrix.m[i][1];  // row i, col 1
		bw << worldMatrix.m[i][2];  // row i, col 2
		bw << worldMatrix.m[i][3];  // row i, col 3
	}
	bw << potapRotation;
	bw << posinRotation;

	header->size = bw.WriteSize();
	header->id = C_MOVEMENT;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
	
}
