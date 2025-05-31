#pragma once
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CComponent : public CBase
{
protected:
	CComponent(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	CComponent(const CComponent& rhs);
	virtual ~CComponent() = default;

public:
	virtual HRESULT Initialize_Prototype() { return S_OK; }
	virtual HRESULT Initialize(void* pArg) { return S_OK; }

protected:
	ID3D12Device* m_Device = { nullptr };
	ID3D12GraphicsCommandList* m_CommandList = { nullptr };

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END