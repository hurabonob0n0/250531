#pragma once

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
	void Update(); // 명령 큐 처리용


	void Send(SendBufferRef sendBuffer);
	void PushCommand(); 


	bool isConnected() { return _connected; };

public:
	int GetMyClientID() { return MyClientID; };

private:
	ClientServiceRef _service;
	bool _connected;
	int MyClientID;

public:
	static	Network_Manager* m_pInstance;
};

