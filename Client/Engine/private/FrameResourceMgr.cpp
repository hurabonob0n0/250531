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

	// GPU가 이 FrameResource를 아직 사용 중이면 기다림
	if (m_MainFence->GetCompletedValue() < frame->Get_Fence_Value())
	{
		HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		m_MainFence->SetEventOnCompletion(frame->Get_Fence_Value(), eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
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

void CFrameResourceMgr::Set_CurrentFramePBMats()
{
	Get_Current_FrameResource()->Set_PassConstants();
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
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

		// GPU가 새 펜스 지점에 도달했으면 이벤트를 발동시킵니다.
		if (FAILED(m_MainFence->SetEventOnCompletion(m_MainFenceValue, eventHandle)))
			MSG_BOX("Failed to Create : Event");

		// GPU가 새 펜스를 설정하고 이벤트가 발동될때까지 기다립니다.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
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
	Safe_Release(m_CommandList);
	Safe_Release(m_CommandQueue);
}
