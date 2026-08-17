#pragma once
#include <condition_variable>

class FJobQueue;

// ================================================================
//  CRoomJobPool - 실행권을 잡은 방을 대신 처리하는 스레드 풀
// ================================================================

class CRoomJobPool
{
public:
	static CRoomJobPool* Get_Instance();
	static void          Destroy_Instance();

public:
	// threadCount <= 0 이면 코어 수와 방 개수 중 작은 값으로 정한다.
	void Start(int32 threadCount = 0);
	void Stop();

	// 실행권을 쥔 잡 큐를 대기열에 넣는다.
	void AddReadyRoom(FJobQueue* pQueue);

	int32 GetThreadCount()  const { return static_cast<int32>(_threads.size()); }
	int32 GetWaitingRoomCount();

private:
	CRoomJobPool() = default;
	~CRoomJobPool();

	void WorkerLoop();

private:
	std::mutex               _lock;
	std::condition_variable  _cv;

	/*  Room 은 Room_Manager 가 기동 때 만들고 종료 때까지 지우지 않음. */
	std::deque<FJobQueue*>   _readyRooms;

	std::vector<std::thread> _threads;
	Atomic<bool>             _running{ false };

	static CRoomJobPool* m_pInstance;
};
