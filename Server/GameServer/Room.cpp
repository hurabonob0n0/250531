#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "ClientSession.h"
#include "ServerPacketHandler.h"
#include "AbstractFactory.h"
#include "Tank.h"
#include "ObjectManager.h"
#include "SGlobal.h"

Room GRoom;

Room::Room() 
{
	
}

Room::Room(uint64 Max)
{

}

Room::~Room()
{
}

void Room::Initialize()
{
	WRITE_LOCK;
	ObjectManager::Get_Instance()->Add_Object(OBJ_TANK, CAbstractFactory<Tank>::Create(0, 0, 0));
	ObjectManager::Get_Instance()->Add_Object(OBJ_TANK, CAbstractFactory<Tank>::Create(0, 0, 0));

	//SendBufferRef sendBuffer0 = ServerPacketHandler::Make_S_ROOMCREATED(1);
	//_Players[0]->OwenerSession->Send(sendBuffer0);

	//SendBufferRef sendBuffer1 = ServerPacketHandler::Make_S_ROOMCREATED(2);
	//_Players[1]->OwenerSession->Send(sendBuffer1);
}

void Room::Update()
{
	if (isStart) {
		system("cls");
		READ_LOCK;
		Position OBJ0 = (*ObjectManager::Get_Instance()->Get_List(OBJ_TANK))[0]->GetPos();
		Position OBJ1 = (*ObjectManager::Get_Instance()->Get_List(OBJ_TANK))[1]->GetPos();

		Vec3_Data Pos[2];
		Pos[0] = { OBJ0.PosX, OBJ0.PosY,OBJ0.PosZ };
		Pos[1] = { OBJ1.PosX, OBJ1.PosY,OBJ1.PosZ };
		Vec3_Data Temp0 = { OBJ0.PosX, OBJ0.PosY, OBJ0.PosZ };
		Vec3_Data Temp1 = { OBJ1.PosX, OBJ1.PosY, OBJ1.PosZ };


		cout << "OBJ0" << "   " << OBJ0.PosX << "     " << OBJ0.PosY << "    " << OBJ0.PosZ << endl;
		cout << "OBJ1" << "   " << OBJ1.PosX << "     " << OBJ1.PosY << "    " << OBJ1.PosZ << endl;

		auto sendBuffer1 = ServerPacketHandler::Make_S_PLAYER_MOVED(Temp1);
		_Players[0]->OwenerSession->Send(sendBuffer1);

		auto sendBuffer2 = ServerPacketHandler::Make_S_PLAYER_MOVED(Temp0);
		_Players[1]->OwenerSession->Send(sendBuffer2);
	}

}

void Room::LateUpdate()
{

	//READ_LOCK;
	//Position OBJ0 = ObjectManager::Get_Instance()->GetGameObject(OBJ_TANK, 0)->GetPos();
	//Position OBJ1 = ObjectManager::Get_Instance()->GetGameObject(OBJ_TANK, 1)->GetPos();
	//SendBufferRef sendBuffer = ServerPacketHandler::Make_S_PLAYER_MOVED(OBJ0.PosX, OBJ0.PosY, OBJ0.PosZ, OBJ1.PosX, OBJ1.PosY, OBJ1.PosZ);
	//Broadcast(sendBuffer);

}

void Room::Release()
{
}

void Room::Accept_Player(PlayerRef Player)
{
	WRITE_LOCK;
	_Players[Player->playerID] = Player;
}

void Room::Leave_Player(PlayerRef Player)
{
	WRITE_LOCK;
	_Players.erase(Player->playerID);
}

void Room::Start()
{


	// TODO: Player 인원 확인( 작업)
	// -> 게임 시작 전 이미 맵, 오브젝트들 생성 -> 미리 생성후 이동으로 가져오기
	// -> Player의 선택 여부에 따라 Tank 선택
	// -> 
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (const auto& iter : _Players)
	{
		const PlayerRef& player = iter.second;
		if (player && player->OwenerSession)
			player->OwenerSession->Send(sendBuffer);
	}
}

void Room::ShowPos()
{
	system("cls");
	READ_LOCK;
	Position OBJ0 = (*ObjectManager::Get_Instance()->Get_List(OBJ_TANK))[0]->GetPos();
	Position OBJ1 = (*ObjectManager::Get_Instance()->Get_List(OBJ_TANK))[1]->GetPos();



	//Vec3_Data Pos[2];
	//Pos[0] = { OBJ0 .PosX, OBJ0.PosY,OBJ0.PosZ};
	//Pos[1] = { OBJ1.PosX, OBJ1.PosY,OBJ1.PosZ };
	//SendBufferRef sendBuffer = ServerPacketHandler::Make_S_PLAYER_MOVED(Pos);

	//Broadcast(sendBuffer);

}

bool Room::Check_Full(uint16 MaxPlayer)
{
	READ_LOCK;
	if (_Players.size() >= MaxPlayer) {
		g_Roomfull.store(true);
		return true;
	}

	return false;
}

void Room::MovePlayer(int64 playerID,float x, float y, float z)
{
	WRITE_LOCK;
	(*ObjectManager::Get_Instance()->Get_List(OBJ_TANK))[playerID]->SetPos(x,y,z);

}

void Room::SetPlayerPos(int64 pID, float x, float y, float z)
{
	WRITE_LOCK;
	(*ObjectManager::Get_Instance()->Get_List(OBJ_TANK))[pID]->SetPos(x, y, z);

}

Vec3_Data Room::GetPlayerPos(int64 pID)
{
	READ_LOCK;
	Vec3_Data Temp;
	Temp.X = (*ObjectManager::Get_Instance()->Get_List(OBJ_TANK))[pID]->GetPos().PosX;
	Temp.Y = (*ObjectManager::Get_Instance()->Get_List(OBJ_TANK))[pID]->GetPos().PosY;
	Temp.Z = (*ObjectManager::Get_Instance()->Get_List(OBJ_TANK))[pID]->GetPos().PosZ;

	return Temp;
}

