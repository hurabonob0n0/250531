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
#include "RoomJobPool.h"

/* GetNowMs() 는 ServerConfig.h 에 있다(Tank 의 검증 상태와 같은 시계를 써야 한다). */

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
		/*  승패가 갈린 뒤 3초 뒤 방을 되돌린다. */
		if (isGameEnded)
		{
			// 끝난 판의 물리와 충돌은 더 돌리지 않는다.
			_resetTimer += deltaTime;
			if (_resetTimer >= ROOM_RESET_DELAY)
				ResetRoom();

			break;
		}

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

	// 디버그 콘솔이 볼 값
	UpdateInfoForDisplay();
}

// ----------------------------------------------------------------
//  방 안의 값을 _display 원자값으로 옮긴다.
// ----------------------------------------------------------------
void Room::UpdateInfoForDisplay()
{
	_displayBlueGauge.store(static_cast<int>(blueGauge));
	_displayRedGauge.store(static_cast<int>(redGauge));

	int tankCount = 0;
	if (auto tankList = Room_ObjectManager.Get_List(OBJ_TANK))
		tankCount = static_cast<int>(tankList->size());

	_displayTankCount.store(tankCount);
}

void Room::LateUpdate()
{

	if (CurState == ROOM_INGAME)
	{
		Room_ObjectManager.Late_Update();
		Broadcast_All_TankStates();
		Broadcast_All_DroneState();
	}
}

void Room::Release()
{

}

// ================================================================
//  틱 잡
// ================================================================

// 실행권을 잡았지만 여기서 돌지 않는다. 방 잡 풀에 넘긴다.
void Room::OnReadyToRun()
{
	CRoomJobPool::Get_Instance()->AddReadyRoom(this);
}

void Room::PushTickJob()
{
	/*  이전 틱이 아직 큐에 남아 있으면 건너뛴다  */
	if (_tickPending.exchange(true))
		return;

	Room* pRoom = this;
	PushJob([pRoom]() { pRoom->TickJob(); });
}

void Room::TickJob()
{

	/*   이 틱을 도는 동안 다음 틱이 예약될 수 있어야 틱 주기가 유지.  */
	_tickPending.store(false);

	const int64 nowMs  = GetNowMs();
	int64       lastMs = _lastTickMs.exchange(nowMs);

	if (lastMs == 0)
		lastMs = nowMs;     // 이 방의 첫 틱

	float deltaTime = static_cast<float>(nowMs - lastMs) / 1000.f;

	/*  방이 오래 놀다 깨어나면 dt 가 몇 초짜리로 들어온다.
	    그대로 넣으면 총알이 한 프레임에 맵을 관통하고 점령 게이지가 튄다.  */
	if (deltaTime > 0.25f)
		deltaTime = 0.25f;
	if (deltaTime < 0.f)
		deltaTime = 0.f;

	Update(deltaTime);
	LateUpdate();
}

#pragma region ForReady

void Room::Accept_Player(PlayerRef Player)
{
	
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

}

void Room::Leave_Player(PlayerRef Player)
{

	const uint64 playerID = Player->playerID;

	// 1. 플레이어 목록에서 제거
	auto playerIt = _Players.find(playerID);
	if (playerIt != _Players.end())
		_Players.erase(playerIt);

	// 2. 상태 데이터에서 제거
	auto stateIt = _Player_States.find(playerID);
	if (stateIt != _Player_States.end())
		_Player_States.erase(stateIt);

	/*  인원 카운터는 여기서 내리지 않는다.
	    Room_Manager::Client_LeaveRoom 이 락 안에서 ReturnPlayerSlot() 으로
	    이미 내렸다. 그래야 "지금 이 방에 자리가 있나" 판단이 즉시 정확해진다
	    (이 잡이 실행될 때까지 기다리면 나간 사람이 자리를 계속 차지한다). */

	Player->OwenerSession.reset();
}

bool Room::Change_Player_Info(uint64 playerID, const Room_Ready_Data& newData)
{

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
	_Player_States[PlayerID] = data;

}

bool Room::Check_ClientLoading()
{

	/* 아무도 없는 방은 절대 시작하지 않는다.  */
	if (RoomCurPlayerCnt == 0)
		return false;

	return Wait_LoadingCnt >= RoomCurPlayerCnt;
}

void Room::Clinet_Loading_Finish()
{
	Wait_LoadingCnt++;
}

bool Room::CanStartGame()
{

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

	if (CurState == ROOM_INGAME)
		return false;

	CurState = ROOM_INGAME;
	isStart = true;

	/*  경과 시간의 기준점  */
	_displayGameStartMs.store(GetNowMs());

	return true;

}

// ----------------------------------------------------------------
//  미리 옮겨 둔 _display* 원자값만 읽는다.
// ----------------------------------------------------------------
Room::FRoomLiveInfo Room::GetLiveInfo()
{
	FRoomLiveInfo info;

	info.State     = CurState.load();
	info.BlueGauge = _displayBlueGauge.load();
	info.RedGauge  = _displayRedGauge.load();
	info.TankCount = _displayTankCount.load();

	if (info.State == ROOM_INGAME)
	{
		const int64 startMs = _displayGameStartMs.load();
		if (startMs > 0)
			info.ElapsedSec = static_cast<int>((GetNowMs() - startMs) / 1000);
	}

	return info;
}

void Room::BroadCast_LobbyInfo()
{
	std::vector<Room_Ready_Data> playerStates;
	{


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

	// 포지션 기반으로 그룹화
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


			Room_ObjectManager.Add_Object(OBJ_DRONE, drone);
		}
		tankIndex++;
	}
}


void Room::Change_Tank_INFO(int64 pID, const Matrix4x4& mat, const float& PotapAngle ,const float& PosinAngle)
{
	if (Tank* pTank = GetTankAt(pID))
		pTank->SetTankState(mat, PotapAngle, PosinAngle);
}


void Room::Broadcast_All_TankStates()
{
	std::vector<Tank_INFO> tankStates;

	// 1. 모든 탱크 상태 수집
	{
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
// ----------------------------------------------------------------
void Room::Broadcast(SendBufferRef sendBuffer)
{
	if (!sendBuffer) return;

	std::vector<SessionRef> targets = SnapshotSessions();

	for (SessionRef& session : targets)
		session->Send(sendBuffer);
}

// 보낼 대상 세션만 먼저 모은다(방 잡 안에서 호출).
std::vector<SessionRef> Room::SnapshotSessions()
{
	std::vector<SessionRef> targets;

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

	auto it = _Players.find(playerID);
	if (it == _Players.end() || !it->second)
		return nullptr;

	return it->second->OwenerSession;
}



#pragma region function

void Room::ShowRoomData()
{
	std::cout << "======== ROOM INFO ========" << std::endl;
	/* uint8 을 그대로 흘리면 숫자가 아니라 문자로 찍힌다. 원래도 그랬다. */
	std::cout << "RoomID: " << (int)RoomID
		<< " | Current Players: " << (int)RoomCurPlayerCnt
		<< " / " << (int)RoomMaxPlayerCnt << std::endl;
	std::cout << "Active: " << (isActive ? "True" : "False") << " | State: ";

	switch (CurState.load())
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
		BulletCnt = Room_ObjectManager.Get_List(OBJ_WEAPON)->size();
	}
	cout << "생성된 총알 : " << BulletCnt << " 개" << endl;
}

bool Room::Wait_Full(uint16 MaxPlayer)
{
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
//  인덱스로 탱크/드론 꺼내기 (방 잡 안에서 호출)
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

// ================================================================
//  서버 검증
//
//  클라가 보낸 값을 그대로 저장하던 자리에 세 겹의 검사를 넣었다.
//    거부하면 그냥 무시한다
// ================================================================
bool Room::IsMoveAllowed(Tank* pTank, const Vec3& pos, int64 nowMs)
{
	if (pTank == nullptr)
		return false;

	/*  1) 월드 경계.  */
	if (pos.X < -WORLD_LIMIT_XZ || pos.X > WORLD_LIMIT_XZ ||
		pos.Z < -WORLD_LIMIT_XZ || pos.Z > WORLD_LIMIT_XZ)
	{
		_rejectWorld.fetch_add(1);
		return false;
	}

	// 2) 지형 높이. 지하 이동과 비행을 막는다.
	const float terrainH = Terrain_Manager::GetInstance().Get_Height(pos.X, pos.Z);
	if (pos.Y < terrainH - TERRAIN_ALLOW_BELOW ||
		pos.Y > terrainH + TERRAIN_ALLOW_ABOVE)
	{
		_rejectTerrain.fetch_add(1);
		return false;
	}

	// 3) 이동 속도. 텔레포트와 속도핵을 막는다.
	if (!pTank->CheckMoveSpeed(pos, nowMs))
	{
		_rejectSpeed.fetch_add(1);
		return false;
	}

	return true;
}

Room::FRejectCounters Room::GetRejectCount() const
{
	FRejectCounters c;
	c.Shot    = _rejectShot.load();
	c.Respawn = _rejectRespawn.load();
	c.World   = _rejectWorld.load();
	c.Terrain = _rejectTerrain.load();
	c.Speed   = _rejectSpeed.load();
	return c;
}

void Room::SetTankState(int64 index, const Matrix4x4& mat, const float& PotapAngle, const float& PosinAngle)
{
	if (Tank* pTank = GetTankAt(index))
		pTank->SetTankState(mat, PotapAngle, PosinAngle);
}

void Room::SetTankPosin(int64 index, const float& PotapAngle, const float& PosinAngle) {
	if (Tank* pTank = GetTankAt(index))
		pTank->SetTankOnlyPosin(PosinAngle, PotapAngle);
}

void Room::SetTankPos(int64 index, const Matrix4x4& mat) {

	if (Tank* pTank = GetTankAt(index))
		pTank->SetTankOnlyPos(mat);
}

/* --- 위 세 개의 쿼터니언. 클라가 실제로 쓰는 경로. --- */

void Room::SetTankStateQuat(int64 index, const Vec3& pos, const Quat& rot,
							const float& PotapAngle, const float& PosinAngle,
							const int8* pWheelSag)
{
	Tank* pTank = GetTankAt(index);
	if (!pTank)
		return;

	const int64 nowMs = GetNowMs();

	/*  위치가 거부되면 포탑/포신도 같이 버리지 않는다.
	    포탑은 포수가 보내는 값이라 이동 치트와 무관하다.  */
	if (IsMoveAllowed(pTank, pos, nowMs))
		pTank->SetTankStateQuat(pos, rot, PosinAngle, PotapAngle, pWheelSag);
	else
		pTank->SetTankOnlyPosin(PosinAngle, PotapAngle);
}

void Room::SetTankPosQuat(int64 index, const Vec3& pos, const Quat& rot, const int8* pWheelSag)
{
	Tank* pTank = GetTankAt(index);
	if (!pTank)
		return;

	if (IsMoveAllowed(pTank, pos, GetNowMs()))
		pTank->SetTankOnlyPosQuat(pos, rot, pWheelSag);
}

void Room::SetDroneState(int64 DroneIndex, const Vec3 Pos, float Yaw, float Roll, float Pitch)
{
	if (Drone* pDrone = GetDroneAt(DroneIndex))
		pDrone->SetDroneState(Pos, Yaw, Roll, Pitch);
}

void Room::SetDroneRespawn(int64 index, const Matrix4x4& mat)
{
	if (Drone* pDrone = GetDroneAt(index))
		pDrone->SetSpawn(mat);
}

void Room::SetTankRespawn(int64 index, const Matrix4x4& mat, const float& PotapAngle, const float& PosinAngle)
{
	Tank* pTank = GetTankAt(index);
	if (!pTank)
		return;

	/*  리스폰 대기(5초)  */
	if (pTank->isSpawned())
	{
		_rejectRespawn.fetch_add(1);
		return;     // 살아 있는데 부활 요청 - 무시
	}

	const int64 nowMs = GetNowMs();
	if (nowMs - pTank->GetDeadTimeMs() < RESPAWN_COOLDOWN_MS - COOLDOWN_TOLERANCE_MS)
	{
		_rejectRespawn.fetch_add(1);
		return;
	}

	pTank->SetSpawn(mat, PotapAngle, PosinAngle);
}

Tank_INFO Room::GetTankState(int64 index)
{

	if (Tank* pTank = GetTankAt(index))
		return pTank->GetTankState();

	return Tank_INFO{};
}

void Room::CreateBullet(int8 pID, uint8 tankindex,WEAPON_ID ID, Vec3 Dir, Vec3 Pos)
{

	switch (ID) {

	case WEAPON_NPOTAN:
	{
		Tank* pShooter = GetTankAt(tankindex);
		if (!pShooter)
			return;  

		/*  재장전  */
		const int64 nowMs = GetNowMs();
		if (nowMs - pShooter->GetLastShotMs() < SHOT_COOLDOWN_MS - COOLDOWN_TOLERANCE_MS)
		{
			_rejectShot.fetch_add(1);
			return;
		}
		pShooter->SetLastShotTime(nowMs);

		// 죽은 탱크
		if (!pShooter->isSpawned())
			return;

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
	constexpr int   bombsPerLine = 6;   // 줄당 6개
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

		/*  점이 아니라  선분으로 본다. */
		const Vec3 bulletPrev = bullet->GetPrevPos();
		const Vec3 bulletPos  = bullet->GetPos();

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

			Vec3 hitPos;
			if (!CollisionManager::GetInstance()->CheckCollision_Segment_OBB3D(
					bulletPrev, bulletPos, targetTank->Get_OBB(), &hitPos))
				continue;

			// 이펙트는 관통 지점이 아니라 처음 닿은 표면에 그린다.
			auto effectBuffer = CPacket_Handler::Make_S_WEAPON_HIT(hitPos.X, hitPos.Y, hitPos.Z);
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
				targetTank->SetUnSpawn(GetNowMs());

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

		/*  지형도 선분으로 본다. 점 판정이면 능선을 스치는 탄이 그냥 통과한다. */
		Vec3 hitPos;
		if (CollisionManager::GetInstance()->CheckCollision_Segment_Terrain(
				bullet->GetPrevPos(), bullet->GetPos(), &hitPos))
		{
			bullet->SetDead(); // 총알 제거

			/*  ★ 여기서 지형 충돌마다 탱크 전원의 OBB 를 cout 으로 20줄씩 찍고 있었다.
			    총알이 땅에 맞을 때마다 나오니 사격 중에는 거의 매 프레임이고,
			    그게 디버그 콘솔의 고정 표를 아래로 밀어내며 화면을 채웠다.
			    필요한 상태는 대시보드가 방 줄 옆에 그린다(Room::GetLiveInfo).
			    콘솔 출력은 락이 걸린 느린 I/O 라, 게임 루프에서 부르는 것 자체가 손해다.  */

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

	if (blueGauge >= 100.f) OnTeamWin(true);
	else if (redGauge >= 100.f) OnTeamWin(false);
}

void Room::ResetRoom()
{
	/*  방 잡 안에서만 부를 것.
	    Room_ObjectManager.Release() 가 오브젝트를 전부 delete 하므로*/

	CurState = ROOM_WAITTING;
	blueGauge = 0.f;
	redGauge = 0.f;
	isGameEnded = false;
	_resetTimer = 0.f;

	/*  게이지 브로드캐스트 기준값도 되돌린다. 안 되돌리면 다음 판에서
	    점령률이 지난 판 최고치를 넘기 전까지 S_CAPTURE 가 안 나간다. */
	lastSentBlueGauge = 0;
	lastSentRedGauge = 0;

	Room_ObjectManager.Release(); // 모든 오브젝트 제거

	Wait_LoadingCnt = 0;
	for (auto& tankState : _Player_States)
		tankState.second.IsReady = false;

	/*  공개 스냅샷도 같이 되돌린다.   */
	_displayBlueGauge.store(0);
	_displayRedGauge.store(0);
	_displayTankCount.store(0);
	_displayGameStartMs.store(0);

	_lastTickMs.store(0);
}

void Room::OnTeamWin(bool isBlueWinner)
{
	if (isGameEnded)
		return;

	isGameEnded = true;

	// 승/패는 팀별로 다른 패킷이라 먼저 <세션, 이긴 팀인가> 로 모아 둔다.
	std::vector<std::pair<SessionRef, bool>> targets;
	targets.reserve(_Players.size());

	for (const auto& pair : _Players)
	{
		const PlayerRef& player = pair.second;
		if (!player || !player->OwenerSession) continue;

		auto stateIt = _Player_States.find(player->playerID);
		if (stateIt == _Player_States.end()) continue;

		targets.emplace_back(player->OwenerSession, stateIt->second.Team == isBlueWinner);
	}

	SendBufferRef winMsg  = CPacket_Handler::Make_S_GAME_WIN(1);
	SendBufferRef loseMsg = CPacket_Handler::Make_S_GAME_LOSE(1);

	for (auto& target : targets)
		target.first->Send(target.second ? winMsg : loseMsg);


	/*  초기화는 여기서 하지 않는다. isGameEnded 를 세워 두면
	    Room::Update 가 ROOM_RESET_DELAY 만큼 세고 ResetRoom 을 부른다.
	    (그래야 방 잡 안에서, 아무도 이 방을 안 만지는 시점에 실행된다) */
	_resetTimer = 0.f;
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

		const Vec3 bombPrev = bomb->GetPrevPos();
		const Vec3 bombPos  = bomb->GetPos();

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

			// 팀 구분 없음. 판정은 총알과 같은 선분 ↔ OBB.
			Vec3 bombHitPos;
			if (!CollisionManager::GetInstance()->CheckCollision_Segment_OBB3D(
					bombPrev, bombPos, targetTank->Get_OBB(), &bombHitPos))
				continue;

			// 1) 이펙트 브로드캐스트
			{
				auto effectBuffer = CPacket_Handler::Make_S_WEAPON_HIT(bombHitPos.X, bombHitPos.Y, bombHitPos.Z);
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
				targetTank->SetUnSpawn(GetNowMs());

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

		Vec3 hitPos;
		if (CollisionManager::GetInstance()->CheckCollision_Segment_Terrain(
				bomb->GetPrevPos(), bomb->GetPos(), &hitPos))
		{
			bomb->SetDead();

			auto sendBuffer = CPacket_Handler::Make_S_WEAPON_HIT(hitPos.X, hitPos.Y, hitPos.Z);
			Broadcast(sendBuffer);
		}
	}
}