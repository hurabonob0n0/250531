#pragma once
#include "ObjectManager.h"
#include "JobQueue.h"

class Tank;

enum ROOM_STATE {

	ROOM_UNACTIVATE, ROOM_WAITTING, ROOM_INGAME, ROOM_END

};


// ================================================================
// 방끼리는 서로를 건드리지 않는다.
// ================================================================
class Room : public FJobQueue
{
public:
	Room();
	Room(uint64 Max);
	~Room();

public:
	void Initialize();
	void Update(float deltaTime);
	void LateUpdate();
	void Release();

public:
	// ---- 틱 ----
	// 게임 루프 스레드가 60Hz 로 부른다. 실행하지 않고 잡만 넣는다.
	void PushTickJob();
	// 그 잡의 본체. 방의 잡 큐 안에서 실행된다.
	void TickJob();

protected:
	// 실행권을 잡았을 때 자기를 방 잡 풀의 대기열에 등록한다.
	virtual void OnReadyToRun() override;

public:
	// ================================================================
	//  입장 슬롯 예약
	// ===============================================================
	void TakePlayerSlot() { ++RoomCurPlayerCnt; }
	void ReturnPlayerSlot() { if (RoomCurPlayerCnt > 0) --RoomCurPlayerCnt; }

public:
	//for PlayerManagement
	// ★ 아래 네 개는 전부 방 잡 안에서만 부를 것(방 명단을 만진다).
	void Accept_Player(PlayerRef Player);
	void Leave_Player(PlayerRef Player);
	bool Change_Player_Info(uint64 playerID, const Room_Ready_Data& newData);

	bool Ready_Player(uint64 playerID);
public:

	void Broadcast_GameStart();
public:
	
	void SpawnTanks();

	void Change_Tank_INFO(int64 pID, const Matrix4x4& mat,
		const float& PosinAngle, const float& PotapAngl);

public:

	bool CollisionTest();
	
	
public:
	//for data
	void Broadcast_PlayerData(Vec3 p1, Vec3 p2);
	void Broadcast_All_TankState(uint8 PlayersCnt);
	void Broadcast_All_DroneState();
	void Broadcast_Hit_Weapon(Vec3 Pos);
	void Broadcast(SendBufferRef sendBuffer);
	void Broadcast_Tank_Data();

	// 전송 대상만 락 안에서 뽑아 오는 헬퍼.
	// 락을 쥔 채 Send 하면 실패 시 Disconnect -> Leave_Player 로 이어져
	// 같은 락을 다시 잡게 되므로, 뽑기와 보내기를 반드시 분리한다.
	std::vector<SessionRef>      SnapshotSessions();
	SessionRef                   FindSession(uint64 playerID);
	std::vector<Room_Ready_Data> GetPassengersOf(uint8 tankIndex);

	// 인덱스로 탱크/드론 꺼내기. 없거나 범위를 벗어나면 nullptr.
	// 인덱스는 클라가 보낸 값이라 반드시 이걸 거쳐야 한다(방 잡 안에서 호출).
	Tank*  GetTankAt(int64 index);
	class Drone* GetDroneAt(int64 index);

public:
	// ================================================================
	//  서버 검증
	// ================================================================

	// 클라가 보낸 위치가 받아들일 수 있는 값인가.
	bool IsMoveAllowed(Tank* pTank, const Vec3& pos, int64 nowMs);

	struct FRejectCounters
	{
		uint32 Shot    = 0;
		uint32 Respawn = 0;
		uint32 World   = 0;
		uint32 Terrain = 0;
		uint32 Speed   = 0;
		uint32 Total() const { return Shot + Respawn + World + Terrain + Speed; }
	};
	FRejectCounters GetRejectCount() const;



public:
	//for Debug
	void ShowRoomData();
	void ShowTankState(uint8 Id);
	void ShowBulletCnt();

public:
	//for PlayerSet
	void SetTankState(int64 Tankindex, const Matrix4x4& mat, const float& PosinAngle, const float& PotapAngl);
	void SetTankPosin(int64 index, const float& PotapAngle, const float& PosinAngle);
	void SetTankPos(int64 index, const Matrix4x4& mat);

	/* 클라가 실제로 쓰는 경로(위치 + 쿼터니언 + 바퀴). 위 행렬판은 스폰 쪽에 남아 있다. */
	void SetTankStateQuat(int64 index, const Vec3& pos, const Quat& rot,
						  const float& PotapAngle, const float& PosinAngle,
						  const int8* pWheelSag);
	void SetTankPosQuat(int64 index, const Vec3& pos, const Quat& rot, const int8* pWheelSag);

	void SetDroneState(int64 DroneIndex, const Vec3 Pos, float Yaw, float Roll, float Pitch);

	void SetDroneRespawn(int64 index, const Matrix4x4& mat);

	Tank_INFO GetTankState(int64 pID);

	void SetTankRespawn(int64 index, const Matrix4x4& mat, const float& PotapAngle, const float& PosinAngle);

public:
	//for GamePlay
	void CreateBullet(int8 playerID,uint8 TankIndex, WEAPON_ID WeaponID, Vec3 Dir, Vec3 Pos);
	void CreateBomb(uint8 playerID, uint8 TankIndex, uint8 AreaNum);
	Tank* FindTankByPlayerId(uint8 playerId);
	void UpdateCaptureGauge(float deltaTime);
	void ResetRoom();
	void OnTeamWin(bool isBlueWinner);
	void Detect_Bomb_Tank_Collisions();
	void Detect_Bomb_Terrain_Collisions();
	void Broadcast_All_TankStates();
	void Detect_Bullet_Tank_Collisions();
	void Send_RespawnPacket(uint8 tankIndex);
	void Send_SoundData(uint8 tnakIndex, float engvol,float engpit, float trkvol,float trkpit);
	void Send_PingData(uint8 tankIndex, float engvol, float engpit, float trkvol);
	void Detect_Bullet_Terrain_Collisions();


public:
	struct FRoomLiveInfo
	{
		ROOM_STATE	State		= ROOM_UNACTIVATE;
		int			ElapsedSec	= 0;
		int			BlueGauge	= 0;
		int			RedGauge	= 0;
		int			TankCount	= 0;
	};

	// 락을 잡지 않는다. 아래 _display 값만 읽는다.
	FRoomLiveInfo GetLiveInfo();

public:

	ObjectManager Room_ObjectManager;

public:

	void RoomActivate() {
		isActive = true;
	}

	void RoomDeActivate() {
		isActive = false;
	}

	bool GetRoomActivate(){
		return isActive;
	}


	/*  이 세 개는 방밖    */
	int GetRoomPlayerCnt(){
		return RoomCurPlayerCnt;
	}

	// 생성 시점에 한 번만 정해지고 이후 바뀌지 않는다.
	int GetRoomMaxPlayerCnt() {
		return RoomMaxPlayerCnt;
	}

	void SetRoomID(int id){
		RoomID = id;
	}
	
	int GetRoomID(){
		return RoomID;
	}

	void BroadCast_LobbyInfo();


	void ChangeRoomState(ROOM_STATE state);

	bool isStart = false;
	bool isMax = false;

	/*  방 밖에서 읽는 값이라 원자값이어야 한다.
	    Room_Manager 의 거의 모든 함수가 이걸 보고 방을 만질지 정한다.  */
	Atomic<bool> isActive{ false };

	int lastSentBlueGauge = 0;
	int lastSentRedGauge = 0;

	unsigned char RoomID;

private:
	uint8 my_RoomID;

	Atomic<uint8> RoomMaxPlayerCnt{ 8 };
	Atomic<uint8> RoomCurPlayerCnt{ 0 };

	uint8 Wait_LoadingCnt = 0;

	bool waitStartDelay = false;    // 2초 딜레이 시작 여부
	float waitStartTimer = 0.0f;    // 딜레이 타이머 (초)

	//queue<>
private:

	// ================================================================
	//  이제 방을 만지는 모든 경로가 이 방의 잡 큐를 거친다. 잡 큐는 한 번에
	//  한 스레드만 들여보내므로, 아래 자료구조들은 언제나
	//  단일 스레드에서만 접근된다. 락 X
	// ================================================================

	map<uint64, PlayerRef>			_Players;
	map<uint64, Room_Ready_Data>	_Player_States;


public:

	// 게임 루프가 쓰고 디버그 콘솔이 읽는다.
	Atomic<ROOM_STATE> CurState{ ROOM_WAITTING };


	/*------------------
	*	For Lobby
	------------------*/

public:
	//for ReadyGame
	bool Check_ClientLoading();
	void Clinet_Loading_Finish();

	/*  _Players.size() 대신 원자 카운터를 본다. Accept_Player / Leave_Player 가
	    맵과 카운터를 같이 갱신하므로 값은 같고, 방 밖에서 락 없이 읽을 수 있다.  */
	uint32 GetPlayers() {
		return RoomCurPlayerCnt;
	};

	bool isFull() {
		return RoomCurPlayerCnt >= RoomMaxPlayerCnt;
	}

	void SetMaxPlayer(uint8 maxPlayer) {
		RoomMaxPlayerCnt = maxPlayer;

	}

	bool Wait_Full(uint16 MaxPlayer);
	void Set_Player_Lobby_State(Room_Ready_Data data, uint64 PlayerID);
	void Show_Room_Data();

	bool CanStartGame();
	bool StartGame();

private:


	uint8 RedTeam_MaxCount = 4;
	uint8 BlueTeam_MaxCount = 4;

	uint8 RedTeam_CurCount = 0;
	uint8 BlueTeam_CurCount = 0;


	float blueGauge = 0.f;
	float redGauge = 0.f;

	const float captureRadius = 300.f;
	const float gaugePerTankPerSecond = 100.f / 300.f; // = 0.333f

	/*  승패가 갈렸는가.*/
	bool  isGameEnded = false;

	// 승패가 갈린 뒤 방을 되돌리기까지 세는 시간.
	static constexpr float ROOM_RESET_DELAY = 3.f;
	float _resetTimer = 0.f;

	Atomic<int>   _displayBlueGauge{ 0 };
	Atomic<int>   _displayRedGauge{ 0 };
	Atomic<int>   _displayTankCount{ 0 };

	/* 0 이면 아직 게임이 시작되지 않은 것.*/
	Atomic<int64> _displayGameStartMs{ 0 };

	/*  검증 카운터.  */
	Atomic<uint32> _rejectShot{ 0 };
	Atomic<uint32> _rejectRespawn{ 0 };
	Atomic<uint32> _rejectWorld{ 0 };
	Atomic<uint32> _rejectTerrain{ 0 };
	Atomic<uint32> _rejectSpeed{ 0 };

	// 게임 루프에서 Update 끝에 한 번 부른다.
	void UpdateInfoForDisplay();

	// ================================================================
	//  틱 잡 관리
	// ================================================================

	/*  ★ 이전 틱 잡이 아직 처리되지 않았으면 새로 밀지 않는다. 밀린 만큼 버림. */
	Atomic<bool>  _tickPending{ false };

	/*  dt 는 게임 루프가 push 한 시각이 아니라 방이 실제로 실행한 시각으로
	    재야 한다. 잡이 큐에서 기다린 시간까지 포함되어야 물리가 안 어긋난다. */
	Atomic<int64> _lastTickMs{ 0 };
};