#pragma once
#include "Base.h"

BEGIN(Engine)

class CRootSignature : public CBase
{
private:
	CRootSignature() {
		constexpr int kMaxRangeSize = 32;
		mRanges.reserve(kMaxRangeSize);
	};
	virtual ~CRootSignature() = default;

public:
	ID3D12RootSignature* Get() const { return m_RootSignature; }

public:
	CRootSignature* Push(D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility, UINT num32BitValues = 0);
	CRootSignature* PushTable(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT registerSpace, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility);

public:
	CRootSignature* CreateDefaultGraphicsRootSignature1();

public:
	CRootSignature* CreateDefaultGraphicsRootSignature();
	// post process (fullscreen pass): slot 0 = SRV table t0 (input), slot 1 = root constants b0 (4 x 32bit), slot 2 = SRV table t1 (second input), static samplers s0~s6
	CRootSignature* CreatePostProcessRootSignature();
	// deferred lighting pass: slot 0 = G-Buffer+depth table (t0~t4, space2), 1 = PassCB b1, 2 = materials (t0, space1),
	// 3 = cube map t0, 4 = shadow map t1, 5 = root constant b2 (debug mode), 6 = root constant b3 (point light count),
	// 7 = root SRV t2 space1 (point lights, same register as DefaultRS slot 9). Slots 1~4 match DefaultRS so the same binding helpers work
	CRootSignature* CreateDeferredLightingRootSignature();
	 
	vector<D3D12_ROOT_PARAMETER>	mParams{};
	vector<D3D12_DESCRIPTOR_RANGE>	mRanges{};

private:
	ID3D12RootSignature* m_RootSignature = nullptr;

public:
	static CRootSignature* Create();
	void Free();
};

END