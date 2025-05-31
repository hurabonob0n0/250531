#pragma once
#include "Base.h"
#include "RenderObject.h"

BEGIN(Engine)

class ENGINE_DLL CFrameResource : public CBase
{
    CFrameResource();
    CFrameResource(const CFrameResource& rhs) = delete;
    CFrameResource& operator=(const CFrameResource& rhs) = delete;
    virtual~CFrameResource() = default;

public:
    void Initialize(ID3D12Device* device);
    ID3D12CommandAllocator* Get_Current_CmdListAlloc() { return m_CmdListAlloc; }
    UINT64                  Get_Fence_Value() { return m_Fence; }
    void                    Set_Fence_Value(UINT64 fenceValue) { m_Fence = fenceValue; }

public:
    void Set_PassConstants();
    void Set_Materials();

public:
    void Set_ObjectConstantBufferIndex(_uint& CBIndex);
    void Reset_ObjectCBIndex() { UsedObjectCBIndex = 0; }

private:
    ID3D12CommandAllocator* m_CmdListAlloc = nullptr;
    UINT64 m_Fence = 0;

public:
    CUploadBuffer<PassConstants>* m_PassCB = nullptr;
    CUploadBuffer<ObjectConstants>* m_ObjectCB = nullptr;
    CUploadBuffer<MaterialData>* m_MaterialCB = nullptr;

public:
    _uint UsedObjectCBIndex = 0;

public:
    static CFrameResource* Create(ID3D12Device* device);
    void Free();
};

END