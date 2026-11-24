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

	// slot 8: 점광원 개수 (b3). Forward 경로의 Default/PosNor PS 가 점광원을 돌 때 쓴다
	Push(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 3, 0, D3D12_SHADER_VISIBILITY_PIXEL, 1);

	// slot 9: 점광원 StructuredBuffer (t2, space1). 루트 SRV (CLightMgr 가 FrameResource 버퍼에 올린 것)
	Push(D3D12_ROOT_PARAMETER_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL);

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

// Deferred 라이팅 패스용 (풀스크린 삼각형). 셰이더가 Common.hlsl 을 그대로 include 하도록 레지스터를 DefaultRS 와 맞췄다
// slot 1, 2 번호도 DefaultRS 와 같게 둬서 Set_CurrentFramePBMats(0) (PassCB = slot 1, 머티리얼 = slot 2) 를 그대로 쓴다
CRootSignature* CRootSignature::CreateDeferredLightingRootSignature()
{
	// slot 0: G-Buffer 4장 + 깊이 (t0~t4, space2). 힙 엔진 칸 4~8 이 붙어 있어서 테이블 하나로 끝난다
	//         space2 로 둔 이유: space0 의 t0(큐브맵), t1(그림자맵), t2~(gTextureMaps) 와 겹치지 않게
	PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2, GBUFFER_COUNT + 1, D3D12_SHADER_VISIBILITY_PIXEL);

	// slot 1: PassCB (b1). 조명, 카메라 위치, 역 ViewProj, 그림자 행렬
	Push(D3D12_ROOT_PARAMETER_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_ALL);

	// slot 2: 머티리얼 버퍼 (t0, space1). 라이팅 셰이더는 안 쓰지만 Set_CurrentFramePBMats 가 slot 2 에 SRV 를 걸기 때문에 자리를 맞춘다
	Push(D3D12_ROOT_PARAMETER_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);

	// slot 3: 큐브맵 (t0). 반사
	PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);

	// slot 4: 그림자맵 (t1)
	PushTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);

	// slot 5: 루트 상수 (b2) 1개 = gDebugMode. 0 조명 결과, 1 노멀, 2 그림자, 3 알베도, 4 광택, 5 발광, 6 깊이
	Push(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 2, 0, D3D12_SHADER_VISIBILITY_ALL, 1);

	// slot 6: 루트 상수 (b3) 1개 = 이번 프레임 점광원 개수
	Push(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 3, 0, D3D12_SHADER_VISIBILITY_PIXEL, 1);

	// slot 7: 점광원 StructuredBuffer (t2, space1). 루트 SRV 라 디스크립터 힙 칸이 필요 없다 (FrameResource 업로드 버퍼 주소를 바로 건다)
	//         조명 영역 패스는 VS 에서도 읽는다 (조명 위치/반경으로 화면 사각형 계산) → visibility ALL
	//         레지스터는 DefaultRS slot 9 와 같다 (Common.hlsl 의 gPointLights 하나를 Forward/Deferred 가 같이 쓴다)
	Push(D3D12_ROOT_PARAMETER_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_ALL);

	auto staticSamplers = GetStaticSamplers();

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
		MSG_BOX("Failed to Serialize : DeferredLighting RootSignature");

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
