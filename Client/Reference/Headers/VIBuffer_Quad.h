#pragma once
#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Quad : public CVIBuffer
{
protected:
    CVIBuffer_Quad(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
    CVIBuffer_Quad(CVIBuffer_Quad& rhs);
    virtual ~CVIBuffer_Quad() = default;

public:
    HRESULT Initialize_Prototype() override;
    HRESULT Initialize(void* pArg) override;
    HRESULT Render(int instanceNum = 1) override;

public:
    static CVIBuffer_Quad* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
    CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

END