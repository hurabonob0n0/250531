#pragma once

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
	void Update(); // ¸í·É Å¥ Ã³¸®¿ë

	void PushPacket(PacketQueueType type, std::function<void()> handler);
	void Dispatch(PacketQueueType type);

	void Send(SendBufferRef sendBuffer);


	void ClearPacketsByType(PacketQueueType type);
	void ClearAllPackets();

	bool isConnected() { return _connected; };

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


	void SetGamstart() { WRITE_LOCK; GameStart = true; };

	bool GetGameStart() {
		READ_LOCK;
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

private:

	ClientServiceRef _service;
	bool _connected = false;
	int MyClientID;

	bool GameStart;

	Room_Ready_Data myData;

	std::vector<Room_Ready_Data> _RoomPlayers;

	std::queue<QueuedPacket> _packetQueue;
	std::mutex _mutex;

	int MyTankIndex;


	int MyKillCount = 0;
	USE_LOCK;

public:
	static	Network_Manager* m_pInstance;
};
