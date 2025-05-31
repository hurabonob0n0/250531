#pragma once
#include "Base.h"
#include "FrameResource.h"

BEGIN(Engine)

class CFrameResourceMgr : public CBase
{
	DECLARE_SINGLETON(CFrameResourceMgr)

private:
	CFrameResourceMgr();
	virtual ~CFrameResourceMgr() = default;

public:
	ID3D12CommandAllocator* Get_CurrentAllocator() const;
	CFrameResource*			Get_Current_FrameResource();

public:
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* pCommandList, ID3D12CommandQueue* pCommandQueue, UINT frameCount);
	void BeginFrame(); 
	void Reset_CommandList_and_Allocator(ID3D12PipelineState* PSO); //Todo : 나중에 매개변수로 PSO받아서 설정해주기
	void SignalAndAdvance();

public:
	void Set_ObjectConstantBufferIndex(_uint& CBIndex);

public:
	void Set_CurrentFramePBMats();

public:
	void Flush_CommandQueue();

private:
	vector<CFrameResource*> m_FrameResources;
	UINT m_CurrentFrameIndex = 0;
	UINT m_NumFrames = 3;

	ID3D12Fence*	m_MainFence = nullptr;
	UINT64			m_MainFenceValue = 0;

	ID3D12GraphicsCommandList*	m_CommandList = nullptr;
	ID3D12CommandQueue*			m_CommandQueue = nullptr;

public:
	void Free() override;
};

END