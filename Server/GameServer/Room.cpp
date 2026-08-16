#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "Session.h"
#include "Packet_Handler.h"
#include "AbstractFactory.h"
#include "Tank.h"
#include "Normal_Potan.h"
#include "ObjectManager.h"
#include "Terrain_Manager.h"
#include "SGlobal.h"
#include "Collision_Manager.h"
#include "Drone.h"
#include "AirDrop_Bomb.h"

Room::Room() 
{
	SetMaxPlayer(8);
	isActive = false;
}

Room::Room(uint64 Max)
{

}

Room::~Room()
{
}

void Room::Initialize()
{
	CurState = ROOM_UNACTIVATE;
}

void Room::Update(float deltaTime)
{
	// ★ 여기에 system("cls") 가 있었다.
	//   방마다, 매 프레임(초당 60번) 프로세스를 새로 띄우고 있었다.
	//   화면 갱신은 CIOCP_Server::DebugConsoleThread 가 0.5초마다 맡는다.

	switch (CurState) {

	case ROOM_UNACTIVATE:

		break;
	case ROOM_WAITTING:
	{
		if (Check_ClientLoading())
			ChangeRoomState(ROOM_INGAME);
	}
		break;
	case ROOM_INGAME:
	{
		Room_ObjectManager.Update(deltaTime);
		
		UpdateCaptureGauge(deltaTime);
		Detect_Bullet_Tank_Collisions();
		Detect_Bullet_Terrain_Collisions();
		
		Detect_Bomb_Terrain_Collisions();
		Detect_Bomb_Tank_Collisions();

	}
		break;
	default:
		break;
	}

}

void Room::LateUpdate()
{

	if (CurState == ROOM_INGAME)
	{
		Room_ObjectManager.Late_Update();
		Broadcast_All_TankStates();
		Broadcast_All_DroneState();

		// ★ 여기서 매 프레임 탱크/드론 좌표를 전부 cout 으로 찍고 있었다.
		//   콘솔 출력은 락이 걸린 느린 I/O 라 그게 곧 게임 루프의 상한이 됐다.
		//   상태를 눈으로 보려면 ShowRoomData()/ShowTankState() 를 쓸 것.
	}
}

void Room::Release()
{

}

#pragma region ForReady

void Room::Accept_Player(PlayerRef Player)
{
	
	WRITE_LOCK(m_playersLock);
	_Players[Player->playerID] = Player;

	// 현재 팀별 인원 수 계산
	int blueCount = 0;
	int redCount = 0;

	for (const auto& pair : _Player_States)
	{
		if (pair.second.Team) // true == Blue
			blueCount++;
		else
			redCount++;
	}

	// 배정할 팀 결정
	bool assignedTeam = false; // false == Red, true == Blue
	if (_Player_States.empty())
	{
		// 첫 입장자는 무조건 Blue
		assignedTeam = true;
	}
	else
	{
		assignedTeam = (blueCount <= redCount); // 인원이 적은 쪽
	}

	// 포지션 후보군 생성
	std::vector<uint8> usedPositions;
	for (const auto& pair : _Player_States)
		usedPositions.push_back(pair.second.Position);

	uint8 assignedPosition = 0;

	if (assignedTeam) // BLUE 팀: 1 ~ 8
	{
		for (uint8 pos = 1; pos <= 8; ++pos)
		{
			if (std::find(usedPositions.begin(), usedPositions.end(), pos) == usedPositions.end())
			{
				assignedPosition = pos;
				break;
			}
		}
	}
	else // RED 팀: 9 ~ 16
	{
		for (uint8 pos = 9; pos <= 16; ++pos)
		{
			if (std::find(usedPositions.begin(), usedPositions.end(), pos) == usedPositions.end())
			{
				assignedPosition = pos;
				break;
			}
		}
	}

	// 최종 데이터 구성 및 저장
	Room_Ready_Data playerData;
	playerData.PlayerID = static_cast<uint8>(Player->playerID);  // uint64 → uint8 캐스팅 주의
	playerData.Team = assignedTeam;
	playerData.Position = assignedPosition;
	playerData.IsReady = false;

	_Player_States[Player->playerID] = playerData;
	++RoomCurPlayerCnt;
}

void Room::Leave_Player(PlayerRef Player)
{
	WRITE_LOCK(m_playersLock);

	const uint64 playerID = Player->playerID;

	// 1. 플레이어 목록에서 제거
	auto playerIt = _Players.find(playerID);
	if (playerIt != _Players.end())
		_Players.erase(playerIt);

	// 2. 상태 데이터에서 제거
	auto stateIt = _Player_States.find(playerID);
	if (stateIt != _Player_States.end())
		_Player_States.erase(stateIt);

	// 3. 현재 인원 수 감소
	if (RoomCurPlayerCnt > 0)
		--RoomCurPlayerCnt;

	// 4. 플레이어가 모두 나간 경우 → 룸 비활성화

}

bool Room::Change_Player_Info(uint64 playerID, const Room_Ready_Data& newData)
{
	WRITE_LOCK(m_playersLock);

	// 플레이어가 존재하는지 먼저 확인
	auto it = _Player_States.find(playerID);
	if (it == _Player_States.end())
		return false;

	// 요청된 포지션이 이미 다른 사람이 쓰고 있는지 확인
	for (const auto& pair : _Player_States)
	{
		if (pair.first == playerID) continue; // 본인 제외

		if (pair.second.Position == newData.Position)
			return false; // 포지션 충돌
	}

	// 유효하다면 데이터 갱신
	Room_Ready_Data updated = newData;
	updated.PlayerID = static_cast<uint8>(playerID); // ID 보존

	_Player_States[playerID] = updated;

	// 변경 사항 전체 전파
	//BroadCast_LobbyInfo();

	return true;
}

bool Room::Ready_Player(uint64 playerID)
{
	WRITE_LOCK(m_playersLock);

	auto it = _Player_States.find(playerID);
	if (it == _Player_States.end())
		return false;

	// 이미 Ready 상태라면 false 리턴
	if (it->second.IsReady)
		return false;

	// Ready 상태로 설정 후 true 리턴
	it->second.IsReady = true;

	return true;
}

void Room::Set_Player_Lobby_State(Room_Ready_Data data, uint64 PlayerID)
{
	WRITE_LOCK(m_playersLock);
	_Player_States[PlayerID] = data;

}

bool Room::Check_ClientLoading()
{
	READ_LOCK(m_playersLock);
	if (Wait_LoadingCnt >= RoomCurPlayerCnt) {
		return true;
	}
	return false;
}

void Room::Clinet_Loading_Finish()
{
	WRITE_LOCK(m_playersLock);
	Wait_LoadingCnt++;
}

bool Room::CanStartGame()
{
	READ_LOCK(m_playersLock);

	if (_Player_States.empty())
		return false;

	for (const auto& pair : _Player_States)
	{
		if (!pair.second.IsReady)
			return false;
	}

	return true;
}

bool Room::StartGame()
{
	WRITE_LOCK(m_lock);

	if (CurState == ROOM_INGAME)
		return false;

	CurState = ROOM_INGAME;
	isStart = true;

	return true;

}

void Room::BroadCast_LobbyInfo()
{
	std::vector<Room_Ready_Data> playerStates;
	{

	READ_LOCK(m_playersLock);

		for (const auto& pair : _Player_States)
		{
			uint64 playerID = pair.first;
			Room_Ready_Data data = pair.second;

			data.PlayerID = static_cast<uint8>(playerID);
			playerStates.push_back(data);
		}
	}

	SendBufferRef sendBuffer = CPacket_Handler::Make_S_ROOM_PLAYER_STATES(playerStates);
	Broadcast(sendBuffer);
}


#pragma endregion Before Start


void Room::Broadcast_GameStart()
{

	SendBufferRef sendBuffer = CPacket_Handler::Make_S_GAME_START(1);
	Broadcast(sendBuffer);
}



void Room::ChangeRoomState(ROOM_STATE state)
{
	if (state == ROOM_INGAME) {

		StartGame();
		SpawnTanks();
		SendBufferRef sendBuffer = CPacket_Handler::Make_S_ALL_PLAYER_LOADING_FINISH(1);
		Broadcast(sendBuffer);

	}
}



#pragma region ForGamePlay


void Room::SpawnTanks()
{
	std::map<int, std::vector<Room_Ready_Data>> tankMap;

	// 포지션 기반으로 그룹화 (짝수: 포탑, 홀수: 조종수 → 조종수 기준 그룹핑)
	for (const auto& pair : _Player_States)
	{
		const Room_Ready_Data& player = pair.second;
		int key = (player.Position % 2 == 0) ? player.Position - 1 : player.Position;
		tankMap[key].push_back(player);
	}

	int tankIndex = 0;
	for (const auto& tankEntry : tankMap)
	{
		int driverPos = tankEntry.first;
		const std::vector<Room_Ready_Data>& passengers = tankEntry.second;

		bool isBlue = driverPos <= 8;

		float x = 50.f * tankIndex;
		float y = 40.f;
		float z = isBlue ? 50.f : 150.f;

		Matrix4x4 tankMat = Matrix4x4::CreateTranslation(x, y, z);
		Matrix4x4 droneMat = Matrix4x4::CreateTranslation(x, y + 50.f, z); // 탱크 위로 오프셋

		// 탱크 생성
		{
			GameObject* tankObj = CAbstractFactory<Tank>::Create();
			Tank* tank = dynamic_cast<Tank*>(tankObj);
			tank->SetBlueTeam(isBlue);

			if (!passengers.empty())
				tank->playerId = passengers[0].PlayerID; // 대표값

			for (const auto& rider : passengers)
				tank->AddPassenger(rider);

			tank->SetTankState(tankMat, 0.f, 0.f); // 기존 시그니처 그대로 사용
			Room_ObjectManager.Add_Object(OBJ_TANK, tank);
		}

		// ── 드론 생성(탱크와 동일 인덱스로 1:1 매칭) ───────────────
		{
			GameObject* droneObj = CAbstractFactory<Drone>::Create();
			Drone* drone = dynamic_cast<Drone*>(droneObj);
			drone->SetBlueTeam(isBlue);

			for (const auto& rider : passengers)
				drone->AddPassenger(rider);

			// 드론도 탱크와 동일한 state 함수가 있다고 했으니 동일하게 호출
			Vec3 Temp = { x, y + 50.f, z };
			drone->SetDroneState(Temp,0,0,0);

			// (선택) 상호 참조 세팅이 있다면 인덱스 교차 기록
			// drone->SetOwnerTankIndex(tankIndex);
			// 또는 tank->SetDroneIndex(tankIndex);

			Room_ObjectManager.Add_Object(OBJ_DRONE, drone);
		}
		tankIndex++;
	}
}


void Room::Change_Tank_INFO(int64 pID, const Matrix4x4& mat, const float& PotapAngle ,const float& PosinAngle)
{
	WRITE_LOCK(m_lock);
	if (Tank* pTank = GetTankAt(pID))
		pTank->SetTankState(mat, PotapAngle, PosinAngle);
}


void Room::Broadcast_All_TankStates()
{
	std::vector<Tank_INFO> tankStates;

	// 1. 모든 탱크 상태 수집
	{
		READ_LOCK(m_lock);
		auto tankList = Room_ObjectManager.Get_List(OBJ_TANK);
		if (tankList)
		{
			for (size_t i = 0; i < tankList->size(); ++i)
			{
				Tank* tank = dynamic_cast<Tank*>((*tankList)[i]);
				if (!tank)
					continue;

				Tank_INFO info = tank->GetTankState();
				tankStates.push_back(info);
			}
		}
	}

	// 2. 패킷 생성 및 모든 플레이어에게 전송
	if (!tankStates.empty())
	{
		auto sendBuffer = CPacket_Handler::Make_S_ALL_TANK_STATE(tankStates);
		Broadcast(sendBuffer);
	}
}

void Room::Broadcast_All_DroneState()
{
	std::vector<Drone_INFO> DroneStates;

	// 1. 모든 드론 상태 수집
	{
		READ_LOCK(m_lock);
		auto DroneList = Room_ObjectManager.Get_List(OBJ_DRONE);
		if (DroneList)
		{
			for (size_t i = 0; i < DroneList->size(); ++i)
			{
				Drone* drone = dynamic_cast<Drone*>((*DroneList)[i]);
				if (!DroneList)
					continue;

				Drone_INFO info = drone->GetDroneState();
				DroneStates.push_back(info);
			}
		}
	}

	// 2. 패킷 생성 및 모든 플레이어에게 전송
	if (!DroneStates.empty())
	{
		auto sendBuffer = CPacket_Handler::Make_S_ALL_DRONE_STATE(DroneStates);
		Broadcast(sendBuffer);
	}
}

void Room::Broadcast_Hit_Weapon(Vec3 Pos)
{
	// 예전에는 여기서 _Players 를 락 없이 돌면서, 수신자마다 똑같은 패킷을
	// 새로 만들고 OwenerSession 을 검사도 없이 역참조했다.
	Broadcast(CPacket_Handler::Make_S_WEAPON_HIT(Pos.X, Pos.Y, Pos.Z));
}


#pragma endregion ForGamePlay


// ----------------------------------------------------------------
//  방 안 전원에게 같은 버퍼를 보낸다.
//
//  ★ 반드시 락을 놓고 나서 Send 한다.
//    Send 가 실패하면 그 자리에서 Disconnect -> Leave_Player 로 이어져
//    m_playersLock 을 다시 잡는다. 락을 쥔 채로 보내면 그 순간 데드락이다.
//    (덤으로, 접속 수만큼의 소켓 호출 동안 다른 스레드를 세워두지 않는다)
// ----------------------------------------------------------------
void Room::Broadcast(SendBufferRef sendBuffer)
{
	if (!sendBuffer) return;

	std::vector<SessionRef> targets = SnapshotSessions();

	for (SessionRef& session : targets)
		session->Send(sendBuffer);
}

// 보낼 대상 세션만 락 안에서 뽑아 온다.
std::vector<SessionRef> Room::SnapshotSessions()
{
	std::vector<SessionRef> targets;

	READ_LOCK(m_playersLock);
	targets.reserve(_Players.size());

	for (const auto& iter : _Players)
	{
		const PlayerRef& player = iter.second;
		if (player && player->OwenerSession)
			targets.push_back(player->OwenerSession);
	}

	return targets;
}

// 특정 플레이어의 세션 하나만 꺼낸다. 없으면 nullptr.
SessionRef Room::FindSession(uint64 playerID)
{
	READ_LOCK(m_playersLock);

	auto it = _Players.find(playerID);
	if (it == _Players.end() || !it->second)
		return nullptr;

	return it->second->OwenerSession;
}



#pragma region function

void Room::ShowRoomData()
{
	std::cout << "======== ROOM INFO ========" << std::endl;
	std::cout << "RoomID: " << (int)RoomID << " | Current Players: " << RoomCurPlayerCnt << " / " << RoomMaxPlayerCnt << std::endl;
	std::cout << "Active: " << (isActive ? "True" : "False") << " | State: ";

	switch (CurState)
	{
	case ROOM_UNACTIVATE:
		break;

	case ROOM_WAITTING: {
		std::cout << "WAITING";
		std::cout << std::endl;
		std::cout << "============================" << std::endl;

		std::cout << "Players in Room:" << std::endl;

		for (const auto& iter : _Players)
		{
			PlayerRef player = iter.second;
			if (player)
			{
				Room_Ready_Data& data = _Player_States[player->playerID];

				std::cout << "PlayerID: " << player->playerID
					<< " | Team: " << (data.Team ? "Blue" : "Red")
					<< " | Position: " << (int)data.Position
					<< " | Loaded: " << (data.IsReady ? "Yes" : "No")
					<< std::endl;
			}
		}
		std::cout << "============================" << std::endl;
	}
		break;
	case ROOM_INGAME:
		std::cout << "INGAME";
		break;
	case ROOM_END:
		std::cout << "END";
		break;
	}
	
}



void Room::ShowTankState(uint8 Id)
{
	// GetTankState 가 이미 m_lock 을 잡는다. 여기서 또 잡으면 자기 자신을 기다린다.
	Tank_INFO Tank0state = GetTankState(Id);
	std::cout << "Tank  "<<  Id  <<"  상태" << std::endl;
	std::cout << "X: " << Tank0state.Pos.X << std::endl;
	std::cout << "Y: " << Tank0state.Pos.Y << std::endl;
	std::cout << "Z: " << Tank0state.Pos.Z << std::endl;
	std::cout << "포탑 각도: " << Tank0state.PotapAngle << std::endl;
	std::cout << "포신 각도: " << Tank0state.PosinAngle << std::endl;
	std::cout << "HP: " << static_cast<int>(Tank0state.TankHP) << std::endl;
}

void Room::ShowBulletCnt()
{

	int BulletCnt = 0;
	if (Room_ObjectManager.Get_List(OBJ_WEAPON)) {
		READ_LOCK(m_lock);
		BulletCnt = Room_ObjectManager.Get_List(OBJ_WEAPON)->size();
	}
	cout << "생성된 총알 : " << BulletCnt << " 개" << endl;
}

bool Room::Wait_Full(uint16 MaxPlayer)
{
	READ_LOCK(m_playersLock);
	int playerCnt = (int)_Players.size();

	std::cout << "플레이어 접속 대기 중" << std::endl;
	std::cout << "접속 플레이어 (" << playerCnt << ") 명" << std::endl;
	std::cout << "최대 플레이어 (" << MaxPlayer << ") 명" << std::endl;

	
	if (playerCnt >= MaxPlayer)
		return true;

	return false;
		
}

#pragma endregion ForDebug

// ================================================================
//  인덱스로 탱크/드론 꺼내기 (m_lock 을 쥔 상태에서 호출)
//
//  ★ 예전에는 전부 이렇게 되어 있었다.
//        dynamic_cast<Tank*>((*Get_List(OBJ_TANK))[index])->SetTankState(...)
//    Get_List 는 리스트가 비어 있으면 nullptr 을 돌려주는데 그걸 그대로
//    역참조했고, index 는 클라가 보낸 값인데 범위 검사도 없었다.
//    그래서 "게임 시작 전(탱크가 아직 없을 때) 이동 패킷 한 방" 또는
//    "말도 안 되는 tankIndex" 로 서버가 그냥 죽었다. 실제로 재현했다.
// ================================================================
Tank* Room::GetTankAt(int64 index)
{
	auto tankList = Room_ObjectManager.Get_List(OBJ_TANK);
	if (!tankList || index < 0 || index >= static_cast<int64>(tankList->size()))
		return nullptr;

	return dynamic_cast<Tank*>((*tankList)[index]);
}

Drone* Room::GetDroneAt(int64 index)
{
	auto droneList = Room_ObjectManager.Get_List(OBJ_DRONE);
	if (!droneList || index < 0 || index >= static_cast<int64>(droneList->size()))
		return nullptr;

	return dynamic_cast<Drone*>((*droneList)[index]);
}

void Room::SetTankState(int64 index, const Matrix4x4& mat, const float& PotapAngle, const float& PosinAngle)
{
	WRITE_LOCK(m_lock);
	if (Tank* pTank = GetTankAt(index))
		pTank->SetTankState(mat, PotapAngle, PosinAngle);
}

void Room::SetTankPosin(int64 index, const float& PotapAngle, const float& PosinAngle) {
	WRITE_LOCK(m_lock);
	if (Tank* pTank = GetTankAt(index))
		pTank->SetTankOnlyPosin(PosinAngle, PotapAngle);
}

void Room::SetTankPos(int64 index, const Matrix4x4& mat) {

	WRITE_LOCK(m_lock);
	if (Tank* pTank = GetTankAt(index))
		pTank->SetTankOnlyPos(mat);
}

/* --- 위 세 개의 쿼터니언 판. 클라가 실제로 쓰는 경로는 이쪽이다. --- */

void Room::SetTankStateQuat(int64 index, const Vec3& pos, const Quat& rot,
							const float& PotapAngle, const float& PosinAngle,
							const int8* pWheelSag)
{
	WRITE_LOCK(m_lock);
	if (Tank* pTank = GetTankAt(index))
		pTank->SetTankStateQuat(pos, rot, PosinAngle, PotapAngle, pWheelSag);
}

void Room::SetTankPosQuat(int64 index, const Vec3& pos, const Quat& rot, const int8* pWheelSag)
{
	WRITE_LOCK(m_lock);
	if (Tank* pTank = GetTankAt(index))
		pTank->SetTankOnlyPosQuat(pos, rot, pWheelSag);
}

void Room::SetDroneState(int64 DroneIndex, const Vec3 Pos, float Yaw, float Roll, float Pitch)
{
	WRITE_LOCK(m_lock);
	if (Drone* pDrone = GetDroneAt(DroneIndex))
		pDrone->SetDroneState(Pos, Yaw, Roll, Pitch);
}

void Room::SetDroneRespawn(int64 index, const Matrix4x4& mat)
{
	WRITE_LOCK(m_lock);
	if (Drone* pDrone = GetDroneAt(index))
		pDrone->SetSpawn(mat);
}

void Room::SetTankRespawn(int64 index, const Matrix4x4& mat, const float& PotapAngle, const float& PosinAngle)
{
	WRITE_LOCK(m_lock);
	if (Tank* pTank = GetTankAt(index))
		pTank->SetSpawn(mat, PotapAngle, PosinAngle);
}

Tank_INFO Room::GetTankState(int64 index)
{
	READ_LOCK(m_lock);

	if (Tank* pTank = GetTankAt(index))
		return pTank->GetTankState();

	return Tank_INFO{};
}

void Room::CreateBullet(int8 pID, uint8 tankindex,WEAPON_ID ID, Vec3 Dir, Vec3 Pos)
{
	WRITE_LOCK(m_lock);

	switch (ID) {

	case WEAPON_NPOTAN:
	{
		Tank* pShooter = GetTankAt(tankindex);
		if (!pShooter)
			return;     // 아직 탱크가 없거나 엉뚱한 인덱스 - 총알을 만들지 않는다

		bool isBlueTeam = pShooter->isBlueTeam();
		GameObject* TempBullet = CAbstractFactory<Normal_Potan>::Create();
		dynamic_cast<Normal_Potan*>(TempBullet)->SetInitData(Dir, Pos, tankindex ,pID, isBlueTeam);
		Room_ObjectManager.Add_Object(OBJ_WEAPON, TempBullet);

		auto sendBuffer = CPacket_Handler::Make_S_BULLETADD(tankindex, Dir.X, Dir.Y, Dir.Z, Pos.X,Pos.Y,Pos.Z);
		Broadcast(sendBuffer);

	}
	break;

	case WEAPON_NBULLET:
		break;

	default:
		break;


	}
}

void Room::CreateBomb(uint8 playerID, uint8 TankIndex, uint8 AreaNum)
{
	WRITE_LOCK(m_lock);

	if (AreaNum < 1 || AreaNum > 9) return;

	// 전역 X 범위(램핑 기준)
	constexpr float WORLD_MIN_X = -500.f;
	constexpr float WORLD_MAX_X = 500.f;

	// Area 경계
	float minX, maxX, minZ, maxZ;
	switch (AreaNum)
	{
	case 1: minX = -500; maxX = -167; minZ = -500; maxZ = -167; break;
	case 2: minX = -167; maxX = 167; minZ = -500; maxZ = -167; break;
	case 3: minX = 167; maxX = 500; minZ = -500; maxZ = -167; break;

	case 4: minX = -500; maxX = -167; minZ = -167; maxZ = 167; break;
	case 5: minX = -167; maxX = 167; minZ = -167; maxZ = 167; break;
	case 6: minX = 167; maxX = 500; minZ = -167; maxZ = 167; break;

	case 7: minX = -500; maxX = -167; minZ = 167; maxZ = 500; break;
	case 8: minX = -167; maxX = 167; minZ = 167; maxZ = 500; break;
	case 9: minX = 167; maxX = 500; minZ = 167; maxZ = 500; break;
	default: return;
	}

	// 배치 파라미터
	constexpr int   bombsPerLine = 6;   // 줄당 6개 → 총 12개
	constexpr float PAD = 10.f;
	constexpr float Z_OFFSET = 40.f; // 두 줄 간격
	constexpr float BASE_ALT = 200.f; // 세계 좌측 끝에서의 기본 고도
	constexpr float RAMP_ALT = 100.f; // 세계 우측 끝에서 추가되는 고도
	const bool leftToRight = true;       // 비행 방향(좌→우). 반대면 false

	const float leftX = minX + PAD;
	const float rightX = maxX - PAD;
	const float centerZ = (minZ + maxZ) * 0.5f;

	for (int line = 0; line < 2; ++line) // 0: 윗줄, 1: 아랫줄
	{
		const float zLine = centerZ + (line == 0 ? -Z_OFFSET : Z_OFFSET);

		for (int i = 0; i < bombsPerLine; ++i)
		{
			// Area 내부에서 X 등분(고정 위치)
			const float tLocal = (bombsPerLine == 1) ? 0.f : float(i) / float(bombsPerLine - 1);
			const float x = leftX + (rightX - leftX) * tLocal;

			// 전역 X 기준 고도 램핑(지형 무시)
			float tGlobal = (x - WORLD_MIN_X) / (WORLD_MAX_X - WORLD_MIN_X); // 01
			tGlobal = std::clamp(tGlobal, 0.f, 1.f);
			if (!leftToRight) tGlobal = 1.f - tGlobal;

			const float y = BASE_ALT + RAMP_ALT * tGlobal;

			Vec3 pos{ x, y, zLine };

			GameObject* obj = CAbstractFactory<AirDrop_Bomb>::Create();
			if (auto* bomb = dynamic_cast<AirDrop_Bomb*>(obj))
				bomb->SetInitData(playerID, TankIndex, pos);

			Room_ObjectManager.Add_Object(OBJ_BOMB, obj); // 필요시 카테고리 조정
		}
	}

	auto sendBuffer = CPacket_Handler::Make_S_AIRDROP(AreaNum);
	Broadcast(sendBuffer);

}



#pragma region function


void Room::Detect_Bullet_Tank_Collisions()
{
	auto bulletList = Room_ObjectManager.Get_List(OBJ_WEAPON);
	auto tankList = Room_ObjectManager.Get_List(OBJ_TANK);
	if (!bulletList || !tankList) return;

	for (GameObject* objBullet : *bulletList)
	{
		if (!objBullet) continue;

		Normal_Potan* bullet = dynamic_cast<Normal_Potan*>(objBullet);
		if (!bullet || bullet->isHit()) continue;

		Vec3 bulletPos = bullet->GetPos();
		uint8 shooterPlayerID = bullet->GetOwnerID();
		bool shooterTeam = bullet->isBlueTeam();

		for (size_t i = 0; i < tankList->size(); ++i)
		{
			Tank* targetTank = dynamic_cast<Tank*>((*tankList)[i]);
			if (!targetTank) continue;

			// 아군이면 무시
			if (targetTank->isBlueTeam() == shooterTeam)
				continue;

			if (!targetTank->isSpawned()) continue;

			// 충돌 판정
			if (!CollisionManager::GetInstance()->CheckCollision_Point_Sphere(bulletPos, targetTank->GetPos(), 5.0f))
				continue;

			// 충돌 시 처리
			auto effectBuffer = CPacket_Handler::Make_S_WEAPON_HIT(bulletPos.X, bulletPos.Y, bulletPos.Z);
			Broadcast(effectBuffer);

			bullet->SetDead();
			targetTank->Damage(25);

			// 피격자에게 TANK_DAMAGED
			for (const Room_Ready_Data& damagedRider : targetTank->GetPassengers())
			{
				if (SessionRef session = FindSession(damagedRider.PlayerID))
					session->Send(CPacket_Handler::Make_S_TANK_DAMAGED((uint8)i));
			}

			Tank* shooterTank = nullptr;
			{
				const uint8 ownerIdx = bullet->GetOwnerTankIndex();
				if (ownerIdx < tankList->size())
					shooterTank = dynamic_cast<Tank*>((*tankList)[ownerIdx]);
			}

			// 공격자에게 TANK_HIT
			if (shooterTank)
			{
				for (const Room_Ready_Data& shooter : shooterTank->GetPassengers())
				{
					if (SessionRef session = FindSession(shooter.PlayerID))
						session->Send(CPacket_Handler::Make_S_TANK_HIT((uint8)i));
				}
			}

			// 사망 판정
			if (targetTank->IsDead())
			{
				targetTank->SetUnSpawn();

				auto bufferDead = CPacket_Handler::Make_S_TANK_DEAD((uint8)i);
				Broadcast(bufferDead);

				if (shooterTank)
				{
					auto bufferKill = CPacket_Handler::Make_S_TANK_KILL((uint8)i);
					for (const Room_Ready_Data& killer : shooterTank->GetPassengers())
					{
						if (SessionRef session = FindSession(killer.PlayerID))
							session->Send(bufferKill);
					}
				}
			}
			break;
		}
	}
}

// 같은 탱크에 탄 인원(조종수/포수)만 뽑는다.
// 탱크 인덱스는 클라가 보낸 값이라 그대로 배열에 넣으면 안 된다.
std::vector<Room_Ready_Data> Room::GetPassengersOf(uint8 tankIndex)
{
	READ_LOCK(m_lock);

	auto tankList = Room_ObjectManager.Get_List(OBJ_TANK);
	if (!tankList || tankIndex >= tankList->size())
		return {};

	Tank* pOwnerTank = dynamic_cast<Tank*>((*tankList)[tankIndex]);
	if (!pOwnerTank)
		return {};

	return pOwnerTank->GetPassengers();
}

void Room::Send_RespawnPacket(uint8 tankIndex)
{
	SendBufferRef respawnBuf = CPacket_Handler::Make_S_RespawnTank(tankIndex);

	for (const Room_Ready_Data& rider : GetPassengersOf(tankIndex))
	{
		if (SessionRef session = FindSession(rider.PlayerID))
			session->Send(respawnBuf);
	}
}

void Room::Send_SoundData(uint8 tankIndex, float engvol, float engpit, float trkvol, float trkpit)
{
	SendBufferRef soundBuf =
		CPacket_Handler::Make_S_SOUND(tankIndex, engvol, engpit, trkvol, trkpit);

	for (const Room_Ready_Data& rider : GetPassengersOf(tankIndex))
	{
		if (SessionRef session = FindSession(rider.PlayerID))
			session->Send(soundBuf);
	}
}

void Room::Send_PingData(uint8 tankIndex, float X, float Y, float Z)
{
	SendBufferRef pingBuf = CPacket_Handler::Make_S_PINGPOS(tankIndex, X, Y, Z);

	for (const Room_Ready_Data& rider : GetPassengersOf(tankIndex))
	{
		if (SessionRef session = FindSession(rider.PlayerID))
			session->Send(pingBuf);
	}
}

void Room::Detect_Bullet_Terrain_Collisions()
{
	auto bulletList = Room_ObjectManager.Get_List(OBJ_WEAPON);
	if (!bulletList) return;

	for (GameObject* objBullet : *bulletList)
	{
		if (!objBullet) continue;

		Normal_Potan* bullet = dynamic_cast<Normal_Potan*>(objBullet);
		if (!bullet || bullet->isHit()) continue;

		if (CollisionManager::GetInstance()->Check_Terrain_Collision(bullet))
		{

		

			Vec3 hitPos = bullet->GetPos();
			bullet->SetDead(); // 총알 제거

			std::cout << "\n=== 지형 충돌 발생 ===" << std::endl;
			std::cout << "총알 위치: X=" << hitPos.X << " Y=" << hitPos.Y << " Z=" << hitPos.Z << std::endl;

			auto tankList = Room_ObjectManager.Get_List(OBJ_TANK);
			if (tankList)
			{
				std::cout << "--- 탱크 OBB 정보 ---" << std::endl;
				for (size_t i = 0; i < tankList->size(); ++i)
				{
					Tank* tank = dynamic_cast<Tank*>((*tankList)[i]);
					if (!tank) continue;

					OBB obb = tank->Get_OBB();

					std::cout << "[탱크 " << i << "]" << std::endl;
					std::cout << "Center: X=" << obb.center.X << " Y=" << obb.center.Y << " Z=" << obb.center.Z << std::endl;
					std::cout << "HalfSize: X=" << obb.halfSize.X << " Y=" << obb.halfSize.Y << " Z=" << obb.halfSize.Z << std::endl;

					std::cout << "Axis[0] (Right): X=" << obb.axis[0].X << " Y=" << obb.axis[0].Y << " Z=" << obb.axis[0].Z << std::endl;
					std::cout << "Axis[1] (Up):    X=" << obb.axis[1].X << " Y=" << obb.axis[1].Y << " Z=" << obb.axis[1].Z << std::endl;
					std::cout << "Axis[2] (Look):  X=" << obb.axis[2].X << " Y=" << obb.axis[2].Y << " Z=" << obb.axis[2].Z << std::endl;
				}
			}
			auto sendBuffer = CPacket_Handler::Make_S_WEAPON_HIT(hitPos.X, hitPos.Y, hitPos.Z);
			Broadcast(sendBuffer);
		}
	}
}


#pragma endregion for_Collision



Tank* Room::FindTankByPlayerId(uint8 playerId)
{
	auto tankList = Room_ObjectManager.Get_List(OBJ_TANK);
	if (!tankList) return nullptr;

	for (GameObject* obj : *tankList)
	{
		Tank* tank = dynamic_cast<Tank*>(obj);
		if (tank && tank->playerId == playerId)
			return tank;
	}
	return nullptr;
}

void Room::UpdateCaptureGauge(float deltaTime)
{
	int blueCount = 0;
	int redCount = 0;

	Vec2 center(0.f, 0.f);
	auto tankList = Room_ObjectManager.Get_List(OBJ_TANK);
	if (!tankList) return;

	for (GameObject* obj : *tankList)
	{
		Tank* tank = dynamic_cast<Tank*>(obj);
		if (!tank || !tank->isSpawned()) continue;

		Vec3 pos3D = tank->GetPos();
		Vec2 pos2D(pos3D.X, pos3D.Z);

		Vec2 offset = pos2D - center;
		if (offset.LengthSq() <= captureRadius * captureRadius)
		{
			if (tank->isBlueTeam()) blueCount++;
			else redCount++;
		}
	}

	// 점령률 누적
	blueGauge += blueCount * gaugePerTankPerSecond * deltaTime;
	redGauge += redCount * gaugePerTankPerSecond * deltaTime;

	//정수 단위로 증가했는지 감지
	int currBlueInt = static_cast<int>(blueGauge);
	int currRedInt = static_cast<int>(redGauge);

	bool shouldBroadcast = false;

	if (currBlueInt > lastSentBlueGauge)
	{
		lastSentBlueGauge = currBlueInt;
		shouldBroadcast = true;
	}

	if (currRedInt > lastSentRedGauge)
	{
		lastSentRedGauge = currRedInt;
		shouldBroadcast = true;
	}

	if (shouldBroadcast)
	{
		auto buffer = CPacket_Handler::Make_S_CAPTURE(blueGauge, redGauge);
		Broadcast(buffer);
	}

	// 디버깅 출력
	std::cout << "점령률 → BLUE: " << blueGauge << " / RED: " << redGauge << std::endl;

	if (blueGauge >= 100.f) OnTeamWin(true);
	else if (redGauge >= 100.f) OnTeamWin(false);
}

void Room::ResetRoom()
{
	// 락 순서는 항상 m_lock -> m_playersLock 이다(반대로 잡는 곳이 없어야 한다).
	{
		WRITE_LOCK(m_lock);

		CurState = ROOM_WAITTING;
		blueGauge = 0.f;
		redGauge = 0.f;
		isGameEnded = false;

		Room_ObjectManager.Release(); // 모든 오브젝트 제거
	}

	{
		WRITE_LOCK(m_playersLock);
		Wait_LoadingCnt = 0;
		for (auto& tankState : _Player_States)
			tankState.second.IsReady = false;
	}
}

void Room::OnTeamWin(bool isBlueWinner)
{
	isGameEnded = true;

	// 승/패는 팀별로 다른 패킷이라 먼저 팀 정보를 락 안에서 뽑아 두고,
	// 전송은 락 밖에서 한다(Broadcast 와 같은 이유).
	std::vector<std::pair<SessionRef, bool>> targets;   // <세션, 승리팀인가>
	{
		READ_LOCK(m_playersLock);
		targets.reserve(_Players.size());

		for (const auto& pair : _Players)
		{
			const PlayerRef& player = pair.second;
			if (!player || !player->OwenerSession) continue;

			auto stateIt = _Player_States.find(player->playerID);
			if (stateIt == _Player_States.end()) continue;

			targets.emplace_back(player->OwenerSession, stateIt->second.Team == isBlueWinner);
		}
	}

	SendBufferRef winMsg  = CPacket_Handler::Make_S_GAME_WIN(1);
	SendBufferRef loseMsg = CPacket_Handler::Make_S_GAME_LOSE(1);

	for (auto& target : targets)
		target.first->Send(target.second ? winMsg : loseMsg);

	// ★ 여기에 system("PAUSE") 가 있었다.
	//   게임 루프 스레드가 콘솔 입력을 기다리며 멈춰 서서,
	//   승패가 갈리는 순간 서버 전체가 정지했다.

	// 3초 후 초기화 예약
	std::thread([this]()
		{
			std::this_thread::sleep_for(std::chrono::seconds(3));
			this->ResetRoom();
		}).detach();
}


void Room::Detect_Bomb_Tank_Collisions()
{
	auto bombList = Room_ObjectManager.Get_List(OBJ_BOMB); // Bomb 전용 컨테이너 권장
	auto tankList = Room_ObjectManager.Get_List(OBJ_TANK);
	if (!bombList || !tankList) return;

	for (GameObject* objBomb : *bombList)
	{
		if (!objBomb) continue;

		AirDrop_Bomb* bomb = dynamic_cast<AirDrop_Bomb*>(objBomb);
		if (!bomb || bomb->isHit()) continue;

		Vec3 bombPos = bomb->GetPos();

		// Bomb의 오너 정보 사용 (아군/적군 무관 타격)
		const uint8 ownerPlayerID = bomb->GetOwnerID();
		const uint8 ownerTankIdx = bomb->GetOwnerTankIndex();

		// (선택) 공격자 탱크 포인터 미리 확보
		Tank* shooterTank = nullptr;
		if (ownerTankIdx < (uint8)tankList->size())
			shooterTank = dynamic_cast<Tank*>((*tankList)[ownerTankIdx]);

		for (size_t i = 0; i < tankList->size(); ++i)
		{
			Tank* targetTank = dynamic_cast<Tank*>((*tankList)[i]);
			if (!targetTank) continue;
			if (!targetTank->isSpawned()) continue;

			// 팀 구분 없음: 포인트-스피어 간단 충돌
			if (!CollisionManager::GetInstance()->CheckCollision_Point_Sphere(bombPos, targetTank->GetPos(), 5.0f))
				continue;

			// 1) 이펙트 브로드캐스트
			{
				auto effectBuffer = CPacket_Handler::Make_S_WEAPON_HIT(bombPos.X, bombPos.Y, bombPos.Z);
				Broadcast(effectBuffer);
			}

			// 2) Bomb 제거(한 번 맞으면 끝)
			bomb->SetDead();

			// 3) 데미지 적용 (원하는 수치로)
			targetTank->Damage(25);

			// 4) 피격자(탑승자 전원)에게 TANK_DAMAGED
			for (const Room_Ready_Data& damagedRider : targetTank->GetPassengers())
			{
				if (SessionRef session = FindSession(damagedRider.PlayerID))
					session->Send(CPacket_Handler::Make_S_TANK_DAMAGED((uint8)i));
			}

			// 5) 사망 시: DEAD 브로드캐스트 + (있다면) 오너에게 KILL
			if (targetTank->IsDead())
			{
				targetTank->SetUnSpawn();

				auto bufferDead = CPacket_Handler::Make_S_TANK_DEAD((uint8)i);
				Broadcast(bufferDead);

				if (shooterTank)
				{
					auto bufferKill = CPacket_Handler::Make_S_TANK_KILL((uint8)i);
					for (const Room_Ready_Data& killer : shooterTank->GetPassengers())
					{
						if (SessionRef session = FindSession(killer.PlayerID))
							session->Send(bufferKill);
					}
				}
			}

			break; // 한 폭탄으로 하나 맞췄으면 종료
		}
	}
}


void Room::Detect_Bomb_Terrain_Collisions()
{
	auto bombList = Room_ObjectManager.Get_List(OBJ_BOMB);
	if (!bombList) return;

	for (GameObject* objBomb : *bombList)
	{
		if (!objBomb) continue;

		AirDrop_Bomb* bomb = dynamic_cast<AirDrop_Bomb*>(objBomb);
		if (!bomb || bomb->isHit()) continue;

		if (CollisionManager::GetInstance()->Check_Terrain_Collision(bomb))
		{
			Vec3 hitPos = bomb->GetPos();
			bomb->SetDead();

			auto sendBuffer = CPacket_Handler::Make_S_WEAPON_HIT(hitPos.X, hitPos.Y, hitPos.Z);
			Broadcast(sendBuffer);
		}
	}
}