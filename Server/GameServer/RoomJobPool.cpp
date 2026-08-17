#include "pch.h"
#include "RoomJobPool.h"
#include "JobQueue.h"

CRoomJobPool* CRoomJobPool::m_pInstance = nullptr;

CRoomJobPool* CRoomJobPool::Get_Instance()
{
	if (!m_pInstance)
		m_pInstance = new CRoomJobPool;
	return m_pInstance;
}

void CRoomJobPool::Destroy_Instance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

CRoomJobPool::~CRoomJobPool()
{
	Stop();
}

void CRoomJobPool::Start(int32 threadCount)
{
	if (_running.exchange(true))
		return;     // 이미 돌고 있다

	if (threadCount <= 0)
	{
		const int32 hw = static_cast<int32>(std::thread::hardware_concurrency());
		threadCount = (hw > 0) ? hw : 4;
		if (threadCount > MAX_ROOM)
			threadCount = MAX_ROOM;
		if (threadCount < 1)
			threadCount = 1;
	}

	_threads.reserve(threadCount);
	for (int32 i = 0; i < threadCount; ++i)
		_threads.emplace_back(&CRoomJobPool::WorkerLoop, this);
}

void CRoomJobPool::Stop()
{
	if (!_running.exchange(false))
		return;

	_cv.notify_all();

	for (std::thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}
	_threads.clear();

	std::lock_guard<std::mutex> lock(_lock);
	_readyRooms.clear();
}

void CRoomJobPool::AddReadyRoom(FJobQueue* pQueue)
{
	if (pQueue == nullptr)
		return;

	{
		std::lock_guard<std::mutex> lock(_lock);
		_readyRooms.push_back(pQueue);
	}

	// 자고 있는 스레드 하나만 깨운다. 방 하나에 스레드 하나면 충분하다.
	_cv.notify_one();
}

int32 CRoomJobPool::GetWaitingRoomCount()
{
	std::lock_guard<std::mutex> lock(_lock);
	return static_cast<int32>(_readyRooms.size());
}

void CRoomJobPool::WorkerLoop()
{
	while (true)
	{
		FJobQueue* pQueue = nullptr;

		{
			std::unique_lock<std::mutex> lock(_lock);

			_cv.wait(lock, [this]()
				{
					return !_readyRooms.empty() || !_running.load();
				});

			// 종료 중이라도 남은 잡은 다 비우고 나간다.
			if (_readyRooms.empty())
				return;

			pQueue = _readyRooms.front();
			_readyRooms.pop_front();
		}

		if (pQueue)
			pQueue->RunJobs();
	}
}
