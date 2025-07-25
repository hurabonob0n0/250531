#include "Client_pch.h"
#include "Client_Globals.h"
#include "Tank.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "ServiceManager.h"
#include "GameInstance.h"
#include "Room_Manager.h"
#include "Level_Manager.h"
#include "Network_Manager.h"

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

	case S_ROOM_DATA:
		Handle_S_GET_ROOMDATA(buffer, len);
		break;
	
	case S_ROOM_ENTER:
		Handle_S_ROOM_ENTER(buffer, len);
		break;

	case S_SUCCESS_ENTER_ROOM:
		Handle_S_SUCCESS_ENTER_ROOM(buffer, len);
		break;

	case S_ROOM_PLAYER_STATES:
		Handle_S_ROOM_PLAYER_STATES(buffer, len);
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


#pragma region ForLobby

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

	std::vector<uint8_t> data(buffer, buffer + len);


	Network_Manager::GetInstance()->PushPacket(PacketQueueType::LOBBY, [data]() {
		BufferReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		PacketHeader header;
		br >> header;
		uint16 ID;
		br >> ID;

		ServiceManager::GetInstace().SetMyID(ID);
		g_PlayerID.store(ID);
		g_ServerConnected.store(true);
		});
}

void ClientPacketHandler::Handle_S_GAME_START(BYTE* buffer, int32 len)
{
	//BufferReader br(buffer, len);

	//PacketHeader header;
	//br >> header;

	//g_GameStart.store(true);


	std::vector<uint8_t> data(buffer, buffer + len);

	Network_Manager::GetInstance()->PushPacket(PacketQueueType::LOBBY, [data]() {
		BufferReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		PacketHeader header;
		br >> header;
		uint16 ID;
		br >> ID;

		});

}

void ClientPacketHandler::Handle_S_SUCCESS_ENTER_ROOM(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	uint16 dummy;
	br >> dummy;


}

void ClientPacketHandler::Handle_S_GET_ROOMDATA(BYTE* buffer, int32 len)
{

	//BufferReader br(buffer, len);

	//PacketHeader header;
	//br >> header;

	//uint32 roomCount;
	//br >> roomCount;

	//std::vector<Room_Data> tempList;
	//tempList.reserve(roomCount);

	//for (uint32 i = 0; i < roomCount; ++i)
	//{
	//	Room_Data data;
	//	br >> data;

	//	tempList.push_back(data);
	//	
	//}
	//Room_Manager::Get_Instance()->SetRoomList(tempList);



	std::vector<uint8_t> data(buffer, buffer + len);


	Network_Manager::GetInstance()->PushPacket(PacketQueueType::LOBBY, [data]() {
		BufferReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

		PacketHeader header;
		br >> header;

		uint32 roomCount;
		br >> roomCount;

		std::vector<Room_Data> tempList;
		tempList.reserve(roomCount);

		for (uint32 i = 0; i < roomCount; ++i)
		{
			Room_Data data;
			br >> data;

			tempList.push_back(data);

		}
		Room_Manager::Get_Instance()->SetRoomList(tempList);
		});

}

void ClientPacketHandler::Handle_S_ROOM_ENTER(BYTE* buffer, int32 len)
{

	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	Level_Manager::Get_Instance()->Level_Change(LEVEL_ROOM);




}

void ClientPacketHandler::Handle_S_ROOM_PLAYER_STATES(BYTE* buffer, int32 len)
{

	std::vector<uint8_t> data(buffer, buffer + len);

	Network_Manager::GetInstance()->PushPacket(PacketQueueType::LOBBY, [data]() {
		BufferReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

		PacketHeader header;
		br >> header;

		uint16 playerCount = 0;
		br >> playerCount;

		std::vector<Room_Ready_Data> roomStates;
		roomStates.reserve(playerCount);

		for (uint16 i = 0; i < playerCount; ++i)
		{
			Room_Ready_Data temp;

			br >> temp.PlayerID;
			br >> temp.Position;
			br >> temp.Team;
			br >> temp.IsReady;

			roomStates.push_back(temp);
		}

		if (Level_Manager::Get_Instance()->GetSceneID() != LEVEL_ROOM)
		{
			Level_Manager::Get_Instance()->Level_Change(LEVEL_ROOM);
			
		}

		Room_Manager::Get_Instance()->SetRoomPlayerStates(roomStates);

		});

}


#pragma endregion Packet_from_Server

#pragma region ForIngame

/*----------------------------
* 
*		For GamePlay
* 
-----------------------------*/


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


#pragma endregion Packet_from_Server


#pragma region ForLobby


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


SendBufferRef ClientPacketHandler::Make_C_SHOWROOM(uint8 Dummy)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << Dummy;

	header->size = bw.WriteSize();
	header->id = C_SHOW_ROOM;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;

}

SendBufferRef ClientPacketHandler::Make_C_JOINROOM(uint32 RoomNum)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << RoomNum;

	header->size = bw.WriteSize();
	header->id = C_JOIN_ROOM;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_CREATEROOM(uint8 Dummy)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << Dummy;

	header->size = bw.WriteSize();
	header->id = C_CREATE_ROOM;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_EXITROOM(uint8 Dummy)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << Dummy;

	header->size = bw.WriteSize();
	header->id = C_EXIT_ROOM;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_CHANGE_INFO(Room_Ready_Data data)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << data.PlayerID << data.Position << data.Team << data.IsReady;
	
	header->size = bw.WriteSize();
	header->id = C_CHANGE_INFO;


	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_READY(uint8 dummy)
{


	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	header->size = bw.WriteSize();
	header->id = C_READY;


	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_START(uint8 dummy)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();


	header->size = bw.WriteSize();
	header->id = C_START;


	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}


#pragma endregion Packet_to_Server

#pragma region ForIngame

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



SendBufferRef ClientPacketHandler::Make_C_MOVE(float x, float y, float z)
{

	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << x << y << z;

	header->size = bw.WriteSize();
	header->id = C_MOVEMENT;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;

}

#pragma endregion Packet_to_Server