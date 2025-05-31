#pragma once
#include "Base.h"

BEGIN(Engine)

template<typename T>
class CUploadBuffer : public CBase
{
public:
    CUploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
        :m_IsConstantBuffer(isConstantBuffer)
    {
        m_ElementByteSize = sizeof(T);

        // Constant buffer´Â 256 byte alignment ÇÊ¿ä
        if (isConstantBuffer)
            m_ElementByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(T));

        HRESULT hr = device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementCount),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_UploadBuffer));

        if (FAILED(hr))
            MSG_BOX("Failed To Create : UploadBuffer");

        hr = m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData));
        if (FAILED(hr))
            MSG_BOX("Failed To Create : UploadBuffer");
    }

    CUploadBuffer(const CUploadBuffer& rhs) = delete;
    CUploadBuffer& operator=(const CUploadBuffer& rhs) = delete;
    virtual ~CUploadBuffer() = default;
  

    ID3D12Resource* Resource() const
    {
        return m_UploadBuffer;
    }

    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&m_MappedData[elementIndex * m_ElementByteSize], &data, sizeof(T));
    }

private:
    ID3D12Resource* m_UploadBuffer = nullptr;
    BYTE* m_MappedData = nullptr;

    UINT m_ElementByteSize = 0;
    bool m_IsConstantBuffer = false;

public:
    static CUploadBuffer* Create(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) 
    {
        CUploadBuffer* pInstance = new CUploadBuffer(device,elementCount,isConstantBuffer);
        return pInstance;
    }
    void Free() override
    {
        if (m_UploadBuffer)
        {
            m_UploadBuffer->Unmap(0, nullptr);
            Safe_Release(m_UploadBuffer);
        }
        m_MappedData = nullptr;
    }
};

END