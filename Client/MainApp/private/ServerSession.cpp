#include "Client_pch.h"
#include "ClientPacketHandler.h"

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