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
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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

	// slot 4: Texture2D gTextureMaps[10] (t1~t10, space0)
	PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 500, D3D12_SHADER_VISIBILITY_PIXEL);

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

CRootSignature* CRootSignature::Create()
{
	CRootSignature* pInstance = new CRootSignature;
	return pInstance;
}

void CRootSignature::Free()
{
	Safe_Release(m_RootSignature);
}
