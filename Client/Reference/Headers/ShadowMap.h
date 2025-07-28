#pragma once
#include "Base.h"

BEGIN(Engine)

class CShadowMap : public CBase
{
	DECLARE_SINGLETON(CShadowMap)

private:
	CShadowMap();
	virtual ~CShadowMap() = default;

public:
	void Initialize();
	void Set_My_Tank_Index(int Index) { myTankIndex = Index; }
	void Late_Update();

public:
	ID3D12Device* m_Device = nullptr;
	ID3D12GraphicsCommandList* m_CommandList;

	ID3D12Resource* m_Resource;

	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsv;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	XMMATRIX S;

	class CTransform* m_TankTransform;

	int myTankIndex;

public:
	virtual void	Free();
};

END