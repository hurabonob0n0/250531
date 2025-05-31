#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"
#include "ClientPacketHandler.h"

#include <chrono>

using namespace std::chrono;


class ServerSession : public PacketSession
{
public:
	~ServerSession()
	{
		cout << "~ServerSession" << endl;
	}

	virtual void OnConnected() override
	{
		cout << "Connected To Server" << endl;
		SendBufferRef sendBuffer = ClientPacketHandler::Make_C_LOGIN(1001, 100, 10);
		Send(sendBuffer);
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		ClientPacketHandler::HandlePacket(buffer, len);
	}

	virtual void OnSend(int32 len) override
	{
		//cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		//cout << "Disconnected" << endl;
	}
};

class Main_App
{
public:
	Main_App();
	~Main_App();

public:
	void Initialize();
	void Update();
	void Late_Update();
	void Release();

};



int main()
{
	this_thread::sleep_for(1s);

	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<ServerSession>,
		1);

	ASSERT_CRASH(service->Start());

	GThreadManager->Launch([&]()
		{
			while (true)
			{
				service->GetIocpCore()->Dispatch();
			}
	});

	Main_App* Main_app = new Main_App;
	Main_app->Initialize();

	constexpr double frameDurationMs = 1000.0 / 60.0;



	//


	while (true)
	{
		auto frameStart = steady_clock::now();

		Main_app->Update();
		Main_app->Late_Update();


		auto frameEnd = steady_clock::now();
		duration<double, std::milli> elapsed = frameEnd - frameStart;

		double sleepTime = frameDurationMs - elapsed.count();
		if (sleepTime > 0)
			this_thread::sleep_for(milliseconds((int)sleepTime));
	}

	delete Main_app;

	GThreadManager->Join();
}