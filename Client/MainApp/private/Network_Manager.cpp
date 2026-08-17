#include "Client_pch.h"
#include "Network_Manager.h"
#include "ClientPacketHandler.h"

Network_Manager* Network_Manager::m_pInstance = nullptr;

Network_Manager::Network_Manager()
{
	_recvBuf.resize(RECV_BUF_SIZE);
}

Network_Manager::~Network_Manager()
{
	Disconnect();
}

// ================================================================
//  연결
//  지금은 소켓 하나에 수신 스레드 하나다.
// ================================================================
bool Network_Manager::Initialize(const std::wstring& ip, uint16 port)
{
	if (_connected)
		return true;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	_wsaStarted = true;		/* Disconnect 가 WSACleanup 을 딱 한 번만 부르도록 */

	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET)
		return false;

	// 게임 패킷은 전부 수십~수백 바이트라 Nagle 이 켜져 있으면 입력이 밀린다.
	BOOL bNoDelay = TRUE;
	setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY,
		reinterpret_cast<const char*>(&bNoDelay), sizeof(bNoDelay));

	SOCKADDR_IN addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	InetPtonW(AF_INET, ip.c_str(), &addr.sin_addr);

	if (connect(_socket,
		reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR)
	{
		closesocket(_socket);
		_socket = INVALID_SOCKET;
		return false;
	}

	_prevRemain = 0;
	_connected = true;

	_recvThread = std::thread(&Network_Manager::RecvThread, this);

	// 접속되자마자 로그인 패킷을 보낸다.
	Send(ClientPacketHandler::Make_C_LOGIN(C_LOGIN));

	return true;
}

void Network_Manager::Disconnect()
{
	_connected = false;

	if (_socket != INVALID_SOCKET)
	{
		// shutdown 을 먼저 해야 수신 스레드의 recv 가 즉시 0 을 돌려주고 빠져나온다.
		shutdown(_socket, SD_BOTH);
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}

	if (_recvThread.joinable())
		_recvThread.join();

	if (_wsaStarted)
	{
		WSACleanup();
		_wsaStarted = false;
	}
}

// ================================================================
//  수신 스레드
// ================================================================
void Network_Manager::RecvThread()
{
	while (_connected)
	{
		int32 received = recv(
			_socket,
			reinterpret_cast<char*>(_recvBuf.data()) + _prevRemain,
			RECV_BUF_SIZE - _prevRemain,
			0);

		if (received <= 0)
		{
			// 0 = 서버가 정상 종료, 음수 = 오류
			_connected = false;
			break;
		}

		ProcessRecvData(received);
	}
}

// ----------------------------------------------------------------
//  패킷 재조립
// ----------------------------------------------------------------
void Network_Manager::ProcessRecvData(int32 newBytes)
{
	int32 remainData = _prevRemain + newBytes;
	BYTE* cursor = _recvBuf.data();

	while (remainData > 0)
	{
		if (remainData < static_cast<int32>(sizeof(PacketHeader)))
			break;

		PacketHeader* header = reinterpret_cast<PacketHeader*>(cursor);

		// 서버가 보낸 값이라도 그대로 믿으면 안 된다.
		if (header->size < sizeof(PacketHeader) ||
			header->size > RECV_BUF_SIZE)
		{
			_connected = false;
			return;
		}

		if (remainData < header->size)
			break;

		ClientPacketHandler::HandlePacket(cursor, header->size);

		cursor += header->size;
		remainData -= header->size;
	}

	_prevRemain = remainData;
	if (remainData > 0)
		memmove(_recvBuf.data(), cursor, remainData);
}

// ================================================================
//  송신
// ================================================================
void Network_Manager::Send(SendBufferRef sendBuffer)
{
	if (!_connected || !sendBuffer)
		return;
	if (sendBuffer->WriteSize() <= 0)
		return;

	std::lock_guard<std::mutex> lock(_sendLock);

	const char* data = reinterpret_cast<const char*>(sendBuffer->Buffer());
	int32       total = sendBuffer->WriteSize();
	int32       sent = 0;

	while (sent < total)
	{
		int32 result = send(_socket, data + sent, total - sent, 0);
		if (result == SOCKET_ERROR || result == 0)
		{
			_connected = false;
			return;
		}
		sent += result;
	}
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

void Network_Manager::ClearPacketsByType(PacketQueueType type)
{
	std::lock_guard<std::mutex> lock(_mutex);
	std::queue<QueuedPacket> tempQueue;

	while (!_packetQueue.empty())
	{
		auto& front = _packetQueue.front();
		if (front.type != type)
			tempQueue.push(front);  // 다른 타입만 남긴다
		_packetQueue.pop();
	}

	std::swap(_packetQueue, tempQueue);
}

void Network_Manager::ClearAllPackets()
{
	std::lock_guard<std::mutex> lock(_mutex);
	while (!_packetQueue.empty())
		_packetQueue.pop();
}
