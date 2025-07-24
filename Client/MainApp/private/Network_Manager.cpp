#include "Client_pch.h"
#include "Network_Manager.h"
#include "ServiceManager.h"
#include "ThreadManager.h"
#include "ClientPacketHandler.h"

Network_Manager* Network_Manager::m_pInstance = nullptr;

class ServerSession : public PacketSession
{
public:

	~ServerSession()
	{

	}

	virtual void OnConnected() override
	{
		SendBufferRef sendBuffer = ClientPacketHandler::Make_C_LOGIN(1001);
		Send(sendBuffer);
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		ClientPacketHandler::HandlePacket(buffer, len);
	}

	virtual void OnSend(int32 len) override
	{

	}

	virtual void OnDisconnected() override
	{

	}

};


Network_Manager::Network_Manager()
{
}

Network_Manager::~Network_Manager()
{
}

bool Network_Manager::Initialize(const std::wstring& ip, uint16 port)
{
	_connected = false;
	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(ip.c_str(), port),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession>,
		1);

	if (!service->Start())
		return false;

	ServiceManager::GetInstace().SetService(service);
	_service = service;

	int32 threadCount = std::thread::hardware_concurrency();
	for (int i = 0; i < threadCount; ++i)
	{
		GThreadManager->Launch([service]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	_connected = true;
	return true;
}

void Network_Manager::Update()
{
}

void Network_Manager::PushPacket(PacketQueueType type, std::function<void()> handler)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_packetQueue.push({ type, handler });
}

void Network_Manager::Dispatch(PacketQueueType type)
{
	std::queue<QueuedPacket> localQueue;

	{
		std::lock_guard<std::mutex> lock(_mutex);
		size_t sz = _packetQueue.size();
		for (size_t i = 0; i < sz; ++i)
		{
			auto& front = _packetQueue.front();
			if (front.type == type)
				localQueue.push(front);
			else
				_packetQueue.push(front);
			_packetQueue.pop();
		}
	}

	while (!localQueue.empty())
	{
		localQueue.front().handler();
		localQueue.pop();
	}
}


void Network_Manager::Send(SendBufferRef sendBuffer)
{
	if (!_connected || _service == nullptr)
		return;
	ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);

}



void Network_Manager::ClearPacketsByType(PacketQueueType type)
{
	std::lock_guard<std::mutex> lock(_mutex);
	std::queue<QueuedPacket> tempQueue;

	while (!_packetQueue.empty())
	{
		auto& front = _packetQueue.front();
		if (front.type != type)
			tempQueue.push(front);  // 다른 타입만 유지
		// 같은 타입이면 버림
		_packetQueue.pop();
	}

	std::swap(_packetQueue, tempQueue);  // 필터링된 큐로 교체
}

void Network_Manager::ClearAllPackets()
{
	std::lock_guard<std::mutex> lock(_mutex);
	while (!_packetQueue.empty())
		_packetQueue.pop();
}