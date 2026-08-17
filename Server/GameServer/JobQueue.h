#pragma once

// ================================================================
//  FJobQueue - 한 번에 한 스레드만을 락 없이 보장
//
//
//  잡 카운트를 0 -> 1 로 올린 스레드 딱 하나만 RunJobs 에 들어간다.
//  그 스레드가 큐를 다 비울 때까지 다른 스레드는 PushJob 만 하고 돌아간다.
//
//  잡 카운트 증가는 반드시 _jobLock 안에서
// ================================================================

using Job = std::function<void()>;

class FJobQueue
{
public:
	virtual ~FJobQueue() = default;

	// ----------------------------------------------------------------
	//  true  : 넣기만 한다. 
	//  false : 실행권을 잡았으면 이 자리에서 큐를 비운다
	//
	//  워커에서는 반드시 true
	// ----------------------------------------------------------------
	void PushJob(Job job, bool bPushOnly = true)
	{
		bool bAcquired = false;
		{
			std::lock_guard<std::mutex> lock(_jobLock);
			_jobs.push_back(std::move(job));
			bAcquired = (_jobCount.fetch_add(1) == 0);
		}

		if (!bAcquired)
			return;     // 이미 누가 실행권을 쥐고 있다. 그가 내 잡까지 소화한다.

		if (bPushOnly)
			OnReadyToRun();
		else
			RunJobs();
	}

	// ----------------------------------------------------------------
	//  실행권을 쥔 스레드만 들어온다.
	//  큐를 통째로 swap 해 가고, 비우는 동안 새로 들어온 잡까지 이어서 처리한다.
	// ----------------------------------------------------------------
	void RunJobs()
	{
		while (true)
		{
			std::vector<Job> batch;
			{
				std::lock_guard<std::mutex> lock(_jobLock);
				batch.swap(_jobs);
			}

			for (Job& job : batch)
				job();

			/*  내가 처리한 개수만큼 뺀다.  */
			const int32 count = static_cast<int32>(batch.size());
			if (_jobCount.fetch_sub(count) == count)
				return;
		}
	}

	// 큐에 잡 밀린 카운트
	int32 GetWaitingJobCount() const { return _jobCount.load(); }

protected:
	// 실행권을 잡았지만 여기서 실행하지 않기로 한 경우 호출
	virtual void OnReadyToRun() {}

private:
	std::mutex       _jobLock;
	std::vector<Job> _jobs;

	// 큐에 남은 잡 개수 겸 실행권
	Atomic<int32>    _jobCount{ 0 };
};
