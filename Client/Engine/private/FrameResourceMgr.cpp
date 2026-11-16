#include "FrameResourceMgr.h"

IMPLEMENT_SINGLETON(CFrameResourceMgr);

CFrameResourceMgr::CFrameResourceMgr()
{
}

void CFrameResourceMgr::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* pCommandList, ID3D12CommandQueue* pCommandQueue, UINT frameCount)
{
	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_MainFence))))
		MSG_BOX("Failed to Create : Fence");

	m_MainFenceValue = 0;

	// 펜스 대기용 이벤트. auto-reset이라 대기가 끝나면 자동으로 비신호 상태로 돌아가 재사용할 수 있다
	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_FenceEvent == nullptr)
		MSG_BOX("Failed to Create : Fence Event");

	m_CurrentFrameIndex = 0;

	m_NumFrames = frameCount;
	m_FrameResources.resize(frameCount);

	for (int i = 0; i < m_NumFrames; ++i)
	{
		m_FrameResources[i] = CFrameResource::Create(device);
		if (m_FrameResources[i] == nullptr)
			return;
	}

	m_CommandList = pCommandList;
	m_CommandQueue = pCommandQueue;

	Safe_AddRef(m_CommandList);
	Safe_AddRef(m_CommandQueue);
}

void CFrameResourceMgr::BeginFrame()
{
	m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_NumFrames;

	auto& frame = m_FrameResources[m_CurrentFrameIndex];

#if defined(_DEBUG)
	// [측정] 펜스 대기 시간 통계. 1초마다 출력 창에 평균을 찍는다.
	static LARGE_INTEGER s_Freq = {};
	static LARGE_INTEGER s_ReportStart = {};
	static double s_WaitSumMs = 0.0;	// 1초 동안 대기한 시간 합
	static double s_WaitMaxMs = 0.0;	// 1초 동안 가장 길었던 대기
	static UINT   s_FrameCount = 0;		// 1초 동안 BeginFrame 호출 수
	static UINT   s_WaitCount = 0;		// 그중 실제로 기다린 프레임 수

	if (s_Freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&s_Freq);
		QueryPerformanceCounter(&s_ReportStart);
	}

	LARGE_INTEGER waitBegin, waitEnd;
	QueryPerformanceCounter(&waitBegin);
	bool bWaited = false;
#endif

	// GPU가 이 FrameResource를 아직 사용 중이면 기다림
	if (m_MainFence->GetCompletedValue() < frame->Get_Fence_Value())
	{
		m_MainFence->SetEventOnCompletion(frame->Get_Fence_Value(), m_FenceEvent);
		WaitForSingleObject(m_FenceEvent, INFINITE);
#if defined(_DEBUG)
		bWaited = true;
#endif
	}

#if defined(_DEBUG)
	QueryPerformanceCounter(&waitEnd);
	const double waitMs = double(waitEnd.QuadPart - waitBegin.QuadPart) * 1000.0 / double(s_Freq.QuadPart);

	s_WaitSumMs += waitMs;
	if (waitMs > s_WaitMaxMs) s_WaitMaxMs = waitMs;
	++s_FrameCount;
	if (bWaited) ++s_WaitCount;

	const double elapsedSec = double(waitEnd.QuadPart - s_ReportStart.QuadPart) / double(s_Freq.QuadPart);
	if (elapsedSec >= 1.0)
	{
		wchar_t buf[256];
		swprintf_s(buf, L"[FenceWait] FrameResources=%u | FPS=%.1f | avg wait=%.3f ms | max wait=%.3f ms | waited %u/%u frames\n",
			m_NumFrames,
			s_FrameCount / elapsedSec,
			s_WaitSumMs / s_FrameCount,
			s_WaitMaxMs,
			s_WaitCount, s_FrameCount);
		OutputDebugStringW(buf);

		s_ReportStart = waitEnd;
		s_WaitSumMs = 0.0;
		s_WaitMaxMs = 0.0;
		s_FrameCount = 0;
		s_WaitCount = 0;
	}
#endif
}

void CFrameResourceMgr::Reset_CommandList_and_Allocator(ID3D12PipelineState* PSO)
{
	auto& frame = m_FrameResources[m_CurrentFrameIndex];

	if (FAILED(frame->Get_Current_CmdListAlloc()->Reset()))
		MSG_BOX("Failed to Reset : Allocator");

	if (FAILED(m_CommandList->Reset(frame->Get_Current_CmdListAlloc(), PSO)))
		MSG_BOX("Failed to Reset : CommandList");
}

void CFrameResourceMgr::SignalAndAdvance()
{
	++m_MainFenceValue;
	m_CommandQueue->Signal(m_MainFence, m_MainFenceValue);
	m_FrameResources[m_CurrentFrameIndex]->Set_Fence_Value(m_MainFenceValue);
	m_FrameResources[m_CurrentFrameIndex]->Reset_ObjectCBIndex();
}

void CFrameResourceMgr::Set_ObjectConstantBufferIndex(_uint& CBIndex)
{
	Get_Current_FrameResource()->Set_ObjectConstantBufferIndex(CBIndex);
}

void CFrameResourceMgr::Set_CurrentFramePBMats(_uint index)
{
	Get_Current_FrameResource()->Set_PassConstants(index);
	Get_Current_FrameResource()->Set_Materials();
}
void CFrameResourceMgr::Flush_CommandQueue()
{
	m_MainFenceValue++;

	// 커맨드들의 처리는 GPU에서 진행되기 때문에 언제 커맨드들이 처리됬는지를 CPU에서 알기 힘듭니다.
	// 그러므로 모든 커맨드가 처리됬을 때 새 펜스 지점을 설정하는 인스트럭션을 커맨드 큐에 추가합니다.
	// Signal()을 호출하기 전에 제출한 커맨드들이 처리되기 전까지 새 펜스 지점은 설정되지 않습니다.
	if (FAILED(m_CommandQueue->Signal(m_MainFence, m_MainFenceValue)))
		MSG_BOX("Failed to Signal : Fence");

	// GPU가 새 펜스 지점까지의 명령들을 완료할 때까지 기다립니다.
	if (m_MainFence->GetCompletedValue() < m_MainFenceValue)
	{
		// GPU가 새 펜스 지점에 도달했으면 이벤트를 발동시킵니다.
		if (FAILED(m_MainFence->SetEventOnCompletion(m_MainFenceValue, m_FenceEvent)))
			MSG_BOX("Failed to Set : Fence Event");

		// GPU가 새 펜스를 설정하고 이벤트가 발동될때까지 기다립니다.
		WaitForSingleObject(m_FenceEvent, INFINITE);
	}
}



ID3D12CommandAllocator* CFrameResourceMgr::Get_CurrentAllocator() const
{
	return m_FrameResources[m_CurrentFrameIndex]->Get_Current_CmdListAlloc();
}

CFrameResource* CFrameResourceMgr::Get_Current_FrameResource()
{
	return m_FrameResources[m_CurrentFrameIndex];
}

void CFrameResourceMgr::Free()
{
	for (auto& frameResource : m_FrameResources)
		Safe_Release(frameResource);
	Safe_Release(m_MainFence);
	if (m_FenceEvent)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}
	Safe_Release(m_CommandList);
	Safe_Release(m_CommandQueue);
}
