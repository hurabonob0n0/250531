#include "RootSignature.h"
#include "GameInstance.h"

CRootSignature* CRootSignature::Push(D3D12_ROOT_PARAMETER_TYPE paramType, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility, UINT num32BitValues)
{
	assert(paramType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);

	D3D12_ROOT_PARAMETER rootParam{};

	rootParam.ParameterType = paramType;
	rootParam.ShaderVisibility = visibility;

	if (num32BitValues > 0) {
		rootParam.Constants.Num32BitValues = num32BitValues;
		rootParam.Constants.ShaderRegister = shaderRegister;
		rootParam.Constants.RegisterSpace = registerSpace;
	}
	else {
		rootParam.Descriptor.ShaderRegister = shaderRegister;
		rootParam.Descriptor.RegisterSpace = registerSpace;
	}

	mParams.push_back(rootParam);

	return this;
}

CRootSignature* CRootSignature::PushTable(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT shaderRegister, UINT registerSpace, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility)
{
	D3D12_DESCRIPTOR_RANGE range{};

	range.RangeType = rangeType;
	range.NumDescriptors = numDescriptors;
	range.BaseShaderRegister = shaderRegister;
	range.RegisterSpace = registerSpace;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	mRanges.push_back(range);


	D3D12_ROOT_PARAMETER rootParam{};

	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.DescriptorTable.NumDescriptorRanges = 1;
	rootParam.DescriptorTable.pDescriptorRanges = &mRanges.back();
	rootParam.ShaderVisibility = visibility;

	mParams.push_back(rootParam);

	return this;
}

CRootSignature* CRootSignature::CreateDefaultGraphicsRootSignature1()
{
	Push(D3D12_ROOT_PARAMETER_TYPE_CBV, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

	// slot 1: PassCB (b1)
	Push(D3D12_ROOT_PARAMETER_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_ALL);


	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = mParams.size();
	rootSignatureDesc.pParameters = mParams.data();
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = rootSignatureFlags;

	ID3DBlob* signatureBlob;
	ID3DBlob* errBlob;

	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errBlob);

	GETDEVICE->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

	Safe_Release(signatureBlob);
	Safe_Release(errBlob);

	return this;

}

CRootSignature* CRootSignature::CreateDefaultGraphicsRootSignature()
{
	// slot 0: ObjectCB (b0)
	Push(D3D12_ROOT_PARAMETER_TYPE_CBV, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

	// slot 1: PassCB (b1)
	Push(D3D12_ROOT_PARAMETER_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_ALL);

	// slot 2: MaterialBuffer (StructuredBuffer<Material> at t0, space1)
	Push(D3D12_ROOT_PARAMETER_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);

	// slot 3: CubeMap Texture (t0, space0)
	PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);

	// slot 4: ShadowMap Texture (t1, space0)
	PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);

	// slot 5: Texture2D gTextureMaps[500] (t2~, space0). MAX_TEXTURE_COUNT 와 셰이더 배열 크기를 맞출 것. 테이블 시작은 힙 SRV_ENGINE_RESERVED 칸
	PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, MAX_TEXTURE_COUNT, D3D12_SHADER_VISIBILITY_PIXEL);

	// slot 6: StructuredBuffer<InstanceBuffer> at t1, space1
	Push(D3D12_ROOT_PARAMETER_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_ALL);
	
	// slot 7: Root Constant (b2)
	Push(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 2, 0, D3D12_SHADER_VISIBILITY_ALL, 1);

	auto staticSamplers = GetStaticSamplers();

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters		= mParams.size();
	rootSignatureDesc.pParameters		= mParams.data();
	rootSignatureDesc.NumStaticSamplers = staticSamplers.size();
	rootSignatureDesc.pStaticSamplers	= staticSamplers.data();
	rootSignatureDesc.Flags				= rootSignatureFlags;

	ID3DBlob* signatureBlob;
	ID3DBlob* errBlob;

	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errBlob);

	GETDEVICE->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

	Safe_Release(signatureBlob);
	Safe_Release(errBlob);

	return this;
}

// 포스트프로세스(풀스크린 삼각형) 패스용. 입력 텍스처 하나 + 작은 상수 몇 개면 충분해서 DefaultRS 와 따로 만든다
// HDR 복사, bright pass, 가우시안 블러, 합성/톤매핑이 모두 이 RS 를 쓴다
CRootSignature* CRootSignature::CreatePostProcessRootSignature()
{
	// slot 0: 입력 텍스처 (t0). HDR RT, Bloom 핑퐁 RT 의 SRV 를 패스마다 바꿔 끼운다
	PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);

	// slot 1: 루트 상수 (b0) 4개. 임계값, 블러 방향, Bloom 강도/노출/톤매핑 모드 같은 패스별 값
	Push(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL, 4);

	// slot 2: 두 번째 입력 텍스처 (t1). 합성 패스에서 Bloom 결과를 읽는다
	// slot 0 테이블을 2칸으로 늘리는 대신 따로 둔 이유: 입력이 하나뿐인 패스(Bloom B 를 읽는 세로 블러 등)에서
	// 테이블 두 번째 칸이 아직 아무 뷰도 안 만든 힙 칸(엔진 4번)을 가리키게 되는 상황을 피하려고
	PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);

	// 정적 샘플러는 DefaultRS 와 같은 s0~s6 (Common.hlsl 과 레지스터 번호 일치)
	auto staticSamplers = GetStaticSamplers();

	// 정점 버퍼를 안 쓰므로 ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT 플래그가 필요 없다
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters		= static_cast<UINT>(mParams.size());
	rootSignatureDesc.pParameters		= mParams.data();
	rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(staticSamplers.size());
	rootSignatureDesc.pStaticSamplers	= staticSamplers.data();
	rootSignatureDesc.Flags				= D3D12_ROOT_SIGNATURE_FLAG_NONE;

	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errBlob);
	if (errBlob)
		OutputDebugStringA(static_cast<const char*>(errBlob->GetBufferPointer()));
	if (SUCCEEDED(hr))
		GETDEVICE->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
	else
		MSG_BOX("Failed to Serialize : PostProcess RootSignature");

	Safe_Release(signatureBlob);
	Safe_Release(errBlob);

	return this;
}

CRootSignature* CRootSignature::Create()
{
	CRootSignature* pInstance = new CRootSignature;
	return pInstance;
}

void CRootSignature::Free()
{
	Safe_Release(m_RootSignature);
}
