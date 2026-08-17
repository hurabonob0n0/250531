#pragma once
#include "Protocol.h"
#include "SendBuffer.h"

// ================================================================
//  Network_Manager
//
//  ★ ServerCore(ClientService + IocpCore + PacketSession + ServerSession)
//    를 걷어내고, 소켓 하나를 이 클래스가 직접 들도록 바꿨다.
//
//    클라는 서버 딱 하나에 붙는다. 연결이 하나뿐이면 IOCP 는 얻는 게 없고
//    (완료 통지를 분배할 상대가 없다) 대신 Service/Session/IocpEvent/
//    RecvBuffer/RefCounting 이 전부 따라온다. 그래서
//        블로킹 소켓 + 전용 수신 스레드 1개 + 락 걸린 send
//    로 줄였다. 서버(IOCP)와 달리 클라는 이게 정답이다.
//
//  ★ 수신 스레드는 UI/렌더를 절대 직접 건드리지 않는다.
//    받은 패킷을 람다로 감싸 _packetQueue 에 넣기만 하고,
//    메인 스레드가 Dispatch() 로 꺼내 실행한다(D3D12 는 이 규칙이 필수).
// ================================================================

enum class PacketQueueType
{
	LOBBY,
	INGAME
};


enum PosMode {

	POS_MASTER, POS_DRIVER, POS_POSU, POS_END

};

enum ControlTarget {

	CONTROL_TANK, CONTROL_POSIN, CONTROL_DRONE, CONTROL_END

};

struct QueuedPacket
{
	PacketQueueType type;
	std::function<void()> handler;
};

class Network_Manager
{
public:
	Network_Manager();
	~Network_Manager();

private:
	
public:
	static Network_Manager* GetInstance()
	{
		if (!m_pInstance)
			m_pInstance = new Network_Manager;
		return m_pInstance;
	}

	static void DestroyInstance()
	{
		if (m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = nullptr;
		}
	}

public:
	bool Initialize(const std::wstring& ip, uint16 port);
	void Disconnect();
	void Update(); // 명령 큐 처리용

	void PushPacket(PacketQueueType type, std::function<void()> handler);
	void Dispatch(PacketQueueType type);

	void Send(SendBufferRef sendBuffer);


	void ClearPacketsByType(PacketQueueType type);
	void ClearAllPackets();

	bool isConnected() { return _connected; };

	// 로그인 응답으로 받은 내 전역 ID.
	// 예전엔 ServiceManager 가 따로 들고 있었는데, 같은 값을 두 군데
	// 저장할 이유가 없어 여기로 합쳤다(SetMyClientID 와 동일한 값이었다).
	void   SetMyID(uint16 id) { _myGlobalID = id; }
	uint16 GetMyID() const    { return _myGlobalID; }

private:
	void RecvThread();                      // 전용 수신 스레드
	void ProcessRecvData(int32 newBytes);   // 헤더 기준 패킷 조립

public:
	int GetMyClientID() {

		return MyClientID;
	};
	void SetMyClientID(int id) {

		MyClientID = id;

	};


	Room_Ready_Data GetMyInGame_Data() {

		for (const auto& data : _RoomPlayers)
		{
			if (data.PlayerID == MyClientID)
				return data;
		}
		return Room_Ready_Data{};

	};

	void SetRoomPlayers(const std::vector<Room_Ready_Data>& players) {

		_RoomPlayers = players;
	}

	const std::vector<Room_Ready_Data>& GetRoomPlayers() const {
		return _RoomPlayers;
	}


	void SetGamstart() {
		std::lock_guard<std::mutex> lock(_stateLock);
		GameStart = true;
	};

	bool GetGameStart() {
		std::lock_guard<std::mutex> lock(_stateLock);
		return GameStart;
	}
	void SetMyTankIndex(int TankIndex) { MyTankIndex = TankIndex; };
	int  GetMyTankIndex() { return MyTankIndex; };

	void Im_damaged() {
		MyHp -= 25;
		if (MyHp <= 0) {
			MyHp = 0;
		}

	};
	void ReSpawn() { MyHp = 100; };

	void add_MyKillCount() {
		++MyKillCount;
	};

	int myPosition;

	bool ReSpawnChoice = false;
	int ReSpawnPos;
	int REDBAR = 0;
	int BLUEBAR = 0;
	int MyHp = 100;


	PosMode MyPosMode;
	ControlTarget MyControlTarget;

	bool PingAdded = false;
private:

	// ---- 소켓 (예전 ClientService + ServerSession) ----
	static constexpr int32 RECV_BUF_SIZE = 8192;

	SOCKET				_socket = INVALID_SOCKET;
	std::atomic<bool>	_connected = false;
	std::thread			_recvThread;

	/*  WSAStartup 을 했는지. _connected 로 대신 판단하면 안 된다 -
		수신 스레드가 스스로 끝나면서 _connected 를 내려버리기 때문이다.  */
	bool				_wsaStarted = false;
	std::mutex			_sendLock;      // send 는 스레드 하나만 들어가야 한다

	// 수신 버퍼. 링버퍼가 아니라, 남은 조각을 배열 앞으로 당겨 이어 붙인다.
	std::vector<BYTE>	_recvBuf;
	int32				_prevRemain = 0;

	int MyClientID;
	uint16 _myGlobalID = 0;

	bool GameStart = false;

	Room_Ready_Data myData;

	std::vector<Room_Ready_Data> _RoomPlayers;

	std::queue<QueuedPacket> _packetQueue;
	std::mutex _mutex;

	int MyTankIndex;


	int MyKillCount = 0;
	std::mutex _stateLock;      // GameStart 등 상태값 보호 (예전 USE_LOCK)

public:
	static	Network_Manager* m_pInstance;
};
