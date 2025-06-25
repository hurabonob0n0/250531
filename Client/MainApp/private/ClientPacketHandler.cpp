#include "Client_pch.h"
#include "Client_Globals.h"
#include "Tank.h"
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

	case S_WEAPON_HIT:
		Handle_S_WEAPON_HIT(buffer, len);
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

void ClientPacketHandler::Handle_S_WEAPON_HIT(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	float X;
	float Y;
	float Z;

	br >> X >> Y >> Z;

	//typedef XMMATRIX					_matrix;

	_vector hitPos = XMVectorSet(X, Y, Z, 1.f);

	// 2. 월드 행렬 생성 (기본 단위 행렬에서 위치만 설정)
	_matrix Hit_Matrix = XMMatrixIdentity();
	Hit_Matrix.r[3] = hitPos;

	//dynamic_cast<Client::CTank*>(CGameInstance::Get_Instance()->GetGameObject("Tank", g_PlayerID.load()))->PushBulletMatrix(Hit_Matrix);
	CGameInstance::Get_Instance()->AddObject("Effect", "Effect", &Hit_Matrix);
}



void ClientPacketHandler::Handle_S_PLAYER_MOVE(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;


	uint8 playerID;
	_float4x4 mat{};
	float potapRot = 0.0f;
	float posinRot = 0.0f;

	br >> playerID;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			br >> mat.m[i][j];
		}
	}

	br >> potapRot;
	br >> posinRot;
	dynamic_cast<Client::CTank*>(CGameInstance::Get_Instance()->GetGameObject("Tank", playerID))->Set_OtherPlayerState(mat, potapRot, posinRot);
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

SendBufferRef ClientPacketHandler::Make_C_FINISH_LOADING(uint64 id)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();
	bw << id;

	header->size = bw.WriteSize();
	header->id = C_FINISH_LOADING;

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

SendBufferRef ClientPacketHandler::Make_C_SHOT(float PosX, float PosY, float PosZ, float nDirX, float nDirY, float nDirZ)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << PosX << PosY << PosZ 
		<< nDirX << nDirY << nDirZ;

	header->size = bw.WriteSize();
	header->id = C_SHOT;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;

}

