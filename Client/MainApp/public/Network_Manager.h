#pragma once

enum class PacketQueueType
{
	LOBBY,
	INGAME
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
	int GetMyClientID() { return MyClientID; };


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

	bool ImPosu = false;
	int myPosition;

	bool ReSpawnChoice = false;
	int ReSpawnPos;
	bool SingleMode = true;

	int REDBAR = 0;
	int BLUEBAR = 0;


private:
	ClientServiceRef _service;
	bool _connected;
	int MyClientID;

	std::queue<QueuedPacket> _packetQueue;
	std::mutex _mutex;

public:
	static	Network_Manager* m_pInstance;
};

