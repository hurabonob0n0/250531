#include "Client_pch.h"
#include "Client_Globals.h"
#include "Tank.h"
#include "Drone.h"
#include "ClientPacketHandler.h"
#include "GameInstance.h"
#include "Room_Manager.h"
#include "Level_Manager.h"
#include "Network_Manager.h"
#include "UIDamaged.h"
#include "UIKill.h"
#include "UISelectPos.h"
#include "UI_DEFEAT.h"
#include "UI_VICTORY.h"
#include "BulletPath.h"
#include "Zet.h"
#include "FMOD_Manager.h"
#include "Camera_Free.h"
#include "BulletPath.h"
#include "Ping.h"

void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	FPacketReader br(buffer, len);

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

	case S_ALL_TANK_STATE:
		Handle_S_ALL_TANK_STATE(buffer, len);
		break;
	case S_ALL_DRONE_STATE:
		Handle_S_ALL_DRONE_STATE(buffer, len);
		break;
	case S_WEAPON_HIT:
		Handle_S_WEAPON_HIT(buffer, len);
		break;

	case S_ROOM_ALL_PLAYER_FINISH_LOADING:
		Handle_S_ROOM_ALL_PLAYER_FINISH_LOADING(buffer, len);
		break;

	case S_TANK_DAMAGED:
		Handle_S_DAMAGED_TANK(buffer, len);
		break;
	case S_TANK_DEAD:
		Handle_S_DEAD_TANK(buffer, len);
		break;

	case S_TANK_HIT:
		Handle_S_HIT_TANK(buffer, len);
		break;

	case S_TANK_KILL:
		Handle_S_KILL_TANK(buffer, len);
		break;

	case S_GAME_WIN:
		Handle_S_GAME_WIN(buffer, len);
		break;

	case S_GAME_LOSE:
		Handle_S_GAME_LOSE(buffer, len);
		break;

	case S_CAPTURE:
		Handle_S_CAPTURE(buffer, len);
		break;

	case S_BULLET_ADD:
		Handle_S_BULLET_ADD(buffer, len);
		break;

	case S_AIRDROP_INDEX:
		Handle_S_AIRDROP(buffer, len);
		break;
	case S_TANK_RESPAWN:
		Handle_S_RESPAWN(buffer, len);
		break;
	case S_TANK_SOUND:
		Handle_S_SOUND(buffer,len);
		break;
	case S_ADD_PING:
		Handle_S_ADD_PING(buffer, len);
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
	FPacketReader br(buffer, len);

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
		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		PacketHeader header;
		br >> header;
		uint16 ID;
		br >> ID;

		Network_Manager::GetInstance()->SetMyID(ID);
		Network_Manager::GetInstance()->SetMyClientID(ID);
		});
}

void ClientPacketHandler::Handle_S_GAME_START(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);

	Network_Manager::GetInstance()->PushPacket(PacketQueueType::LOBBY, [data]() {
		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		PacketHeader header;
		br >> header;
		uint16 ID;
		br >> ID;

		Level_Manager::Get_Instance()->SetGamePlayMode();


		});


}

void ClientPacketHandler::Handle_S_SUCCESS_ENTER_ROOM(BYTE* buffer, int32 len)
{
	FPacketReader br(buffer, len);

	PacketHeader header;
	br >> header;

	uint16 dummy;
	br >> dummy;


}

void ClientPacketHandler::Handle_S_GET_ROOMDATA(BYTE* buffer, int32 len)
{


	std::vector<uint8_t> data(buffer, buffer + len);


	Network_Manager::GetInstance()->PushPacket(PacketQueueType::LOBBY, [data]() {
		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

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

	FPacketReader br(buffer, len);

	PacketHeader header;
	br >> header;

	Level_Manager::Get_Instance()->Level_Change(LEVEL_ROOM);


}

void ClientPacketHandler::Handle_S_ROOM_PLAYER_STATES(BYTE* buffer, int32 len)
{

	std::vector<uint8_t> data(buffer, buffer + len);

	Network_Manager::GetInstance()->PushPacket(PacketQueueType::LOBBY, [data]() {
		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

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
		Network_Manager::GetInstance()->SetRoomPlayers(roomStates);

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

	std::vector<uint8_t> data(buffer, buffer + len);

	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

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

		CGameInstance::Get_Instance()->AddObject("Effect", "Effect", &Hit_Matrix);

		auto* FM = FMOD_Manager::Get_Instance();
		AudioVec3 p{ X, Y, Z };
		AudioVec3 v{ 0, 0, 0 };
		FMOD::Channel* ch = nullptr;
		if (FM->Play3D_ReturnChannel("Explosion", p, v, &ch, /*volume=*/2.f, /*paused=*/false) && ch) {
			float pitch = 0.97f + (rand() / (float)RAND_MAX) * (1.03f - 0.97f); // 0.97~1.03
			ch->setPitch(pitch);
		}

		});
}

void ClientPacketHandler::Handle_S_BULLET_ADD(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		PacketHeader header;
		br >> header;

		uint8 TankIndex;
		float DirX;
		float DirY;
		float DirZ;

		float PosX;
		float PosY;
		float PosZ;

		br>> TankIndex >> DirX >> DirY >> DirZ >> PosX >> PosY >> PosZ;

		CBulletPath::BulletPathstr bps;
		bps.OwnerTankIndex = TankIndex;
		bps.Dir = XMVectorSet(DirX, DirY, DirZ, 0.f);
		bps.Pos = XMVectorSet(PosX, PosY, PosZ, 1.f);
		CGameInstance::Get_Instance()->AddObject("BulletPath", "BulletPath", &bps);


		_matrix mat = XMMatrixTranslation(PosX, PosY + 0.5f, PosZ);
		CGameInstance::Get_Instance()->AddObject("SmokeEffect", "Effect", &mat);

		mat = XMMatrixTranslation(PosX + 0.1f, PosY + 0.6f, PosZ + 0.1f);
		CGameInstance::Get_Instance()->AddObject("SmokeEffect", "Effect", &mat);

		mat = XMMatrixTranslation(PosX - 0.1f, PosY + 0.6f, PosZ - 0.1f);
		CGameInstance::Get_Instance()->AddObject("SmokeEffect", "Effect", &mat);


		auto* FM = FMOD_Manager::Get_Instance();
		AudioVec3 p{ PosX, PosY, PosZ };
		AudioVec3 v{ 0, 0, 0 };
		// 약간의 피치 랜덤으로 더 자연스럽게
		FMOD::Channel* ch = nullptr;
		if (FM->Play3D_ReturnChannel("Tank_Shot", p, v, &ch, /*volume=*/0.4f, /*paused=*/false) && ch) {
			float pitch = 0.98f + (rand() / (float)RAND_MAX) * (1.02f - 0.98f); // 0.98~1.02
			ch->setPitch(pitch);
		}

		if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_TANK && Network_Manager::GetInstance()->GetMyTankIndex() == TankIndex) {
			dynamic_cast<CCamera_Free*>(CGameInstance::Get_Instance()->GetGameObject("Camera", 0))->StartShake(0.7f, 0.7f, 40.f);
		}


		});
}

void ClientPacketHandler::Handle_S_HIT_TANK(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		((CUIKill*)(CGameInstance::Get_Instance()->GetGameObject("UI", UI_DAMAGE)))->set_Hit();
		});
}

void ClientPacketHandler::Handle_S_DAMAGED_TANK(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		Network_Manager::GetInstance()->Im_damaged();
		dynamic_cast<CCamera_Free*>(CGameInstance::Get_Instance()->GetGameObject("Camera", 0))->StartShake(1.3f, 0.9f, 50.f);
	});
}

void ClientPacketHandler::Handle_S_DEAD_TANK(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		PacketHeader header;
		br >> header;
		uint8 index;
		br >> index;

		Client::CTank* tank = dynamic_cast<Client::CTank*>(CGameInstance::Get_Instance()->GetGameObject("Tank", index));
		if (index == Network_Manager::GetInstance()->GetMyTankIndex()) {
			tank->set_Spawn(false);
			tank->_respawnTimer = 0.f;
			((CUISelectPos*)(CGameInstance::Get_Instance()->GetGameObject("UI", UI_SELECT_POS)))->set_render();
			//TODOUI -> UISelectPos UI바꾸기, MasterMode와 Driver가 선택지 선택, POSU일때는 선택 불가능 UI 띄우기

			auto* FM = FMOD_Manager::Get_Instance();
			FMOD::Channel* ch = nullptr;
			if (FM->Play2D_ReturnChannel("DeadSound", &ch, /*volume=*/0.6f, /*paused=*/false) && ch) {
			}
		}
		});
}

void ClientPacketHandler::Handle_S_KILL_TANK(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		((CUIDamaged*)(CGameInstance::Get_Instance()->GetGameObject("UI", UI_KILL)))->set_Hit();
		Network_Manager::GetInstance()->add_MyKillCount();

		auto* FM = FMOD_Manager::Get_Instance();
		FMOD::Channel* ch = nullptr;
		if (FM->Play2D_ReturnChannel("KillSound", &ch, /*volume=*/0.6f, /*paused=*/false) && ch) {
		}

	});
}

void ClientPacketHandler::Handle_S_GAME_WIN(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		dynamic_cast<CUI_VICTORY*>((CGameInstance::Get_Instance()->GetGameObject("UI", UI_VICTORY)))->set_render();
		});
}

void ClientPacketHandler::Handle_S_GAME_LOSE(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		dynamic_cast<CUI_DEFEAT*>((CGameInstance::Get_Instance()->GetGameObject("UI", UI_DEFEAT)))->set_render();
	});
}

void ClientPacketHandler::Handle_S_CAPTURE(BYTE* buffer, int32 len)
{

	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

		PacketHeader header;
		br >> header;
		
		uint8 Blue, red;
		br >> Blue >> red;
		Network_Manager::GetInstance()->REDBAR = red;
		Network_Manager::GetInstance()->BLUEBAR = Blue;

	});
}

void ClientPacketHandler::Handle_S_AIRDROP(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

		PacketHeader header;
		uint8 AreaIndex;
		br >> header;
		br >> AreaIndex;

		dynamic_cast<CZet*>(CGameInstance::Get_Instance()->GetGameObject("Zet", 0))->Set_StartPos_And_Move(AreaIndex);

		auto* FM = FMOD_Manager::Get_Instance();
		FMOD::Channel* ch = nullptr;
		if (FM->Play2D_ReturnChannel("AirDrop", &ch, /*volume=*/0.6f, /*paused=*/false) && ch) {
		}
	});

}

void ClientPacketHandler::Handle_S_RESPAWN(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

		PacketHeader header;
		uint8 TankIndex;
		br >> header;
		br >> TankIndex;
		if (Network_Manager::GetInstance()->MyPosMode == POS_POSU) {
			dynamic_cast<CTank*>(CGameInstance::Get_Instance()->GetGameObject("Tank", TankIndex))->setRespawnForPosinMode();
			((CUISelectPos*)(CGameInstance::Get_Instance()->GetGameObject("UI", UI_SELECT_POS)))->set_render_off();
		}
	});


}

void ClientPacketHandler::Handle_S_SOUND(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

		PacketHeader header;
		uint8 TankIndex;
		float engvol, engpit, trackvol, trackpit;
		br >> header;
		br >> TankIndex;
		br >> engvol >> engpit >> trackvol >> trackpit;
		if (Network_Manager::GetInstance()->MyPosMode == POS_POSU) {
			dynamic_cast<CTank*>(CGameInstance::Get_Instance()->GetGameObject("Tank", TankIndex))->SetSoundData(engvol, engpit, trackvol, trackpit);
		}
	});

}

void ClientPacketHandler::Handle_S_ADD_PING(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));
		PacketHeader header;
		uint8 tankindex; 
		float X, Y, Z;
		br >> header;
		br>>tankindex >> X >> Y >> Z;

		if (Network_Manager::GetInstance()->MyPosMode == POS_DRIVER) {

			CGameInstance::Get_Instance()->AddObject("Ping", "Ping", nullptr);
			int index = CGameInstance::Get_Instance()->GetLayerSize("Ping")- 1;
			dynamic_cast<CPing*>(CGameInstance::Get_Instance()->GetGameObject("Ping", index))->Set_Position(X,Y,Z);

		}

	});


}

void ClientPacketHandler::Handle_S_ROOM_ALL_PLAYER_FINISH_LOADING(BYTE* buffer, int32 len)
{


	FPacketReader br(buffer, len);

	PacketHeader header;
	br >> header;

	Network_Manager::GetInstance()->SetGamstart();


}

void ClientPacketHandler::Handle_S_ALL_TANK_STATE(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);

	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {

		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

		PacketHeader header;
		br >> header;

		uint16 tankCount = 0;
		br >> tankCount;

		for (uint16 i = 0; i < tankCount; ++i)
		{
			uint8 TankIndex;
			br >> TankIndex;


			//위치 3 -> 쿼터니언 4 -> 포탑 -> 포신 -> HP -> 바퀴 14

			_float3 vPos = {};
			_float4 qRot = {};
			br >> vPos.x >> vPos.y >> vPos.z;
			br >> qRot.x >> qRot.y >> qRot.z >> qRot.w;

			float potapAngle = 0.f;
			float posinAngle = 0.f;
			uint8 tankHP = 0;

			br >> potapAngle >> posinAngle >> tankHP;

			int8 wheelSag[TANK_WHEEL_COUNT] = {};
			for (int w = 0; w < TANK_WHEEL_COUNT; ++w)
				br >> wheelSag[w];

			// 모자란 패킷이면 여기서 멈춘다 
			if (br.IsUnderflow())
				break;

			_float4x4 mat = {};
			XMStoreFloat4x4(&mat,
				XMMatrixRotationQuaternion(XMLoadFloat4(&qRot)) *
				XMMatrixTranslation(vPos.x, vPos.y, vPos.z));

			uint8 myTankID = Network_Manager::GetInstance()->GetMyTankIndex();

			Client::CTank* tank = dynamic_cast<Client::CTank*>(CGameInstance::Get_Instance()->GetGameObject("Tank", static_cast<int>(TankIndex)));
			if (TankIndex != myTankID)
			{
				if (tank)
				{
					tank->Set_OtherPlayerState(mat, potapAngle, posinAngle);

					//남의 탱크 바퀴
					tank->Set_WheelSagFromServer(wheelSag);
				}
			}
			else
			{
				if (Network_Manager::GetInstance()->MyPosMode == POS_POSU)
				{
					tank->SetMyMatrix(mat);
					tank->Set_WheelSagFromServer(wheelSag);
				}
				else if(Network_Manager::GetInstance()->MyPosMode == POS_DRIVER)
				{
					tank->Set_DriverModeData(potapAngle, posinAngle);
				}
				else {
				/*  마스터 포지션 - 일부러 비어 있다.
					MASTER 는 탱크에 혼자 탄 경우에만 붙으므로(MainApp.cpp 참고)
					차체도 포탑도 전부 자기가 주인이다. 서버가 돌려준 값을 여기서
					다시 넣으면 자기 조작이 서버 왕복에 밀려 뭉개진다.

					⚠️ 여기에 포탑을 넣어 '포수와 같이 탄 마스터' 를 지원하려 하지 말 것.
					   MASTER 는 RotPotap_And_Posin 으로 자기 카메라를 따라 포탑을 돌리므로
					   같은 값을 두 곳에서 쓰게 되어 서로 덮어쓴다.
					   둘이 탈 때는 DRIVER/POSU 로 나누는 게 맞다.                        */
				}
			}

		}
		});
}

void ClientPacketHandler::Handle_S_ALL_DRONE_STATE(BYTE* buffer, int32 len)
{
	std::vector<uint8_t> data(buffer, buffer + len);
	Network_Manager::GetInstance()->PushPacket(PacketQueueType::INGAME, [data]() {
		
		FPacketReader br(reinterpret_cast<BYTE*>(const_cast<uint8_t*>(data.data())), static_cast<int32>(data.size()));

		PacketHeader header;
		br >> header;

		uint16 DroneCount = 0;
		br >> DroneCount;

		for (uint16 i = 0; i < DroneCount; ++i)
		{
			uint8 DroneIndex;
			float PosX, PosY, PosZ, Yaw, Roll, Pitch;
			uint8 DroneHP = 0;
			br >> DroneIndex;
			br >> PosX >> PosY >> PosZ >> Yaw >> Roll >> Pitch;

			br >> DroneHP;

			uint8 myDroneIndex = Network_Manager::GetInstance()->GetMyTankIndex();
			 
			Client::CDrone* drone = dynamic_cast<Client::CDrone*>(CGameInstance::Get_Instance()->GetGameObject("Drone", static_cast<int>(DroneIndex)));
			if (DroneIndex != myDroneIndex || Network_Manager::GetInstance()->MyPosMode == POS_DRIVER)
			{
				if (drone)
				{
					drone->SetOtherDroneMat(PosX,PosY,PosZ,Yaw,Roll,Pitch);
				}
			}

		}
	});
}


#pragma endregion Packet_from_Server


#pragma region ForLobby


SendBufferRef ClientPacketHandler::Make_C_LOGIN(uint64 id)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();
	bw << id;

	header->size = bw.WriteSize();
	header->id = C_LOGIN;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}


SendBufferRef ClientPacketHandler::Make_C_SHOWROOM(uint8 Dummy)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << Dummy;

	header->size = bw.WriteSize();
	header->id = C_SHOW_ROOM;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;

}

SendBufferRef ClientPacketHandler::Make_C_JOINROOM(uint32 RoomNum)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << RoomNum;

	header->size = bw.WriteSize();
	header->id = C_JOIN_ROOM;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_CREATEROOM(uint8 Dummy)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << Dummy;

	header->size = bw.WriteSize();
	header->id = C_CREATE_ROOM;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_EXITROOM(uint8 Dummy)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << Dummy;

	header->size = bw.WriteSize();
	header->id = C_EXIT_ROOM;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_CHANGE_INFO(Room_Ready_Data data)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << data.PlayerID << data.Position << data.Team << data.IsReady;
	
	header->size = bw.WriteSize();
	header->id = C_CHANGE_INFO;


	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_READY(uint8 dummy)
{


	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	header->size = bw.WriteSize();
	header->id = C_READY;


	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_START(uint8 dummy)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();


	header->size = bw.WriteSize();
	header->id = C_START;


	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}


SendBufferRef ClientPacketHandler::Make_C_LOADING_FINISH(uint8 dummy)
{

	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();


	header->size = bw.WriteSize();
	header->id = C_FINISH_LOADING;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

/*===========================================================================
	탱크 자세를 와이어에 싣는다
===========================================================================*/
static void WriteTankTransform(FPacketWriter& bw, const _float4x4& worldMatrix)
{
	_vector vScale, qRot, vPos;

	if (!XMMatrixDecompose(&vScale, &qRot, &vPos, XMLoadFloat4x4(&worldMatrix)))
	{
		//분해가 안 되는 행렬이면 회전을 포기하고 위치만 살린다
		qRot = XMQuaternionIdentity();
		vPos = XMLoadFloat4x4(&worldMatrix).r[3];
	}

	_float3 vP; XMStoreFloat3(&vP, vPos);
	_float4 vQ; XMStoreFloat4(&vQ, qRot);

	bw << vP.x << vP.y << vP.z;
	bw << vQ.x << vQ.y << vQ.z << vQ.w;
}

static void WriteWheelSag(FPacketWriter& bw, const int8* pWheelSag)
{
	for (int i = 0; i < TANK_WHEEL_COUNT; ++i)
		bw << (pWheelSag ? pWheelSag[i] : (int8)0);
}

SendBufferRef ClientPacketHandler::Make_C_TANK_RESPAWN(_float4x4& worldMatrix, float potapRotation, float posinRotation)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();


	bw << (uint8)Network_Manager::GetInstance()->GetMyTankIndex();
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
	header->id = C_RESPAWN_TANK;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_TANK_POSINMOVE(float potapRotation, float posinRotation)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << (uint8)Network_Manager::GetInstance()->GetMyTankIndex();
	bw << potapRotation;
	bw << posinRotation;

	header->size = bw.WriteSize();
	header->id = C_MYPOSIN;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_TANK_POSMOVE(_float4x4& worldMatrix, const int8* pWheelSag)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << (uint8)Network_Manager::GetInstance()->GetMyTankIndex();
	WriteTankTransform(bw, worldMatrix);
	WriteWheelSag(bw, pWheelSag);

	header->size = bw.WriteSize();
	header->id = C_MYPOS;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_AIRDROP(uint8 AreaNum)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << (uint8)Network_Manager::GetInstance()->GetMyTankIndex();
	bw << (uint8)AreaNum;

	header->size = bw.WriteSize();
	header->id = C_AIRDROP;
	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_SOUND(float EngineVol, float EnginePitch, float TrackVol, float TrackPitch)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << (uint8)Network_Manager::GetInstance()->GetMyTankIndex();
	bw << EngineVol << EnginePitch << TrackVol << TrackPitch;

	header->size = bw.WriteSize();
	header->id = C_TANK_SOUND;
	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}

SendBufferRef ClientPacketHandler::Make_C_PING(float X, float Y, float Z)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << (uint8)Network_Manager::GetInstance()->GetMyTankIndex();
	bw << X << Y << Z;

	header->size = bw.WriteSize();
	header->id = C_ADD_PING;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}

#pragma endregion Packet_to_Server

#pragma region ForIngame

SendBufferRef ClientPacketHandler::Make_C_MOVE(_float4x4& worldMatrix, float potapRotation, float posinRotation, const int8* pWheelSag)
{

	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();


	uint8 Tankindex = Network_Manager::GetInstance()->GetMyTankIndex();
	bw << Tankindex;
	WriteTankTransform(bw, worldMatrix);
	bw << potapRotation;
	bw << posinRotation;
	WriteWheelSag(bw, pWheelSag);

	header->size = bw.WriteSize();
	header->id = C_MOVEMENT;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;

}

SendBufferRef ClientPacketHandler::Make_C_KEYINPUT(uint8 key)
{

	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();
	bw << key;

	header->size = bw.WriteSize();
	header->id = C_KEYINPUT;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}


SendBufferRef ClientPacketHandler::Make_C_SHOT(float PosX, float PosY, float PosZ, float nDirX, float nDirY, float nDirZ)
{
	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	uint8 TankIndex = Network_Manager::GetInstance()->GetMyTankIndex();
	bw << TankIndex <<PosX << PosY << PosZ	<< nDirX << nDirY << nDirZ;

	header->size = bw.WriteSize();
	header->id = C_SHOT;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;

}

SendBufferRef ClientPacketHandler::Make_C_DRONE_MOVE(float PosX, float PosY, float PosZ, float Yaw, float Roll, float Pitch)
{


	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();


	uint8 Droneindex = Network_Manager::GetInstance()->GetMyTankIndex();
	bw << Droneindex;
	bw << PosX << PosY << PosZ << Yaw << Roll << Pitch;


	header->size = bw.WriteSize();
	header->id = C_MYDRONEMOVE;

	sendBuffer->Close(bw.WriteSize());
	return sendBuffer;
}



SendBufferRef ClientPacketHandler::Make_C_MOVE(float x, float y, float z)
{

	SendBufferRef sendBuffer = MakeSendBuffer(4096);
	FPacketWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << x << y << z;

	header->size = bw.WriteSize();
	header->id = C_MOVEMENT;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;

}



#pragma endregion Packet_to_Server