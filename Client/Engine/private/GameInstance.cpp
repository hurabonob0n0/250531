#include "GameInstance.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance() : m_Graphic_Dev(CGraphic_Dev::Get_Instance()),m_FrameResourceMgr(CFrameResourceMgr::Get_Instance())
{
}

void CGameInstance::Initialize(WindowInfo& windowInfo, CRawInput* pRawInput)
{
	//ID3D12Debug* debugController;
	//D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	//debugController->EnableDebugLayer();



	//InputDev
	m_Input_Dev = pRawInput;
	Safe_AddRef(m_Input_Dev);

	//GraphicDev
	m_Graphic_Dev->Initialize(windowInfo);

	//RandomMgr
	m_RandomMgr = CRandom_Manager::Get_Instance();
	m_RandomMgr->Tick();
	
	//FrameResourceMgr + GraphicDev
	m_FrameResourceMgr->Initialize(m_Graphic_Dev->Get_Device(), m_Graphic_Dev->Get_CommandList(), m_Graphic_Dev->Get_CommandQueue(), g_NumFrameResources);
	m_Graphic_Dev->OnResize();
	m_FrameResourceMgr->Flush_CommandQueue();
	m_FrameResourceMgr->Reset_CommandList_and_Allocator(nullptr);

	//ComponentMgr
	m_ComponentMgr = CComponentMgr::Get_Instance();
	m_ComponentMgr->AddPrototype("CBBindingCom", CBBinding::Create(Get_Device(), Get_CommandList(), m_FrameResourceMgr));

	//RootSignatureMgr
	m_RootSignatureMgr = CRootSignatureMgr::Get_Instance();
	m_RootSignatureMgr->Register("DefaultRS", CRootSignature::Create()->CreateDefaultGraphicsRootSignature());
	m_RootSignatureMgr->Register("PostProcessRS", CRootSignature::Create()->CreatePostProcessRootSignature());
	m_RootSignatureMgr->Register("DeferredLightingRS", CRootSignature::Create()->CreateDeferredLightingRootSignature());

	//ShaderMgr
	m_ShaderMgr = CShader_Mgr::Get_Instance();
	m_ShaderMgr->AddShader("DefaultVS", CShader::ST_VS, L"../bin/Shaders1/Default.hlsl", nullptr);
	m_ShaderMgr->AddShader("DefaultPS", CShader::ST_PS, L"../bin/Shaders1/Default.hlsl", nullptr);

	m_ShaderMgr->AddShader("SkyVS", CShader::ST_VS, L"../bin/Shaders1/Sky.hlsl", nullptr);
	m_ShaderMgr->AddShader("SkyPS", CShader::ST_PS, L"../bin/Shaders1/Sky.hlsl", nullptr);

	m_ShaderMgr->AddShader("PosNorVS", CShader::ST_VS, L"../bin/Shaders1/Default3.hlsl", nullptr);
	m_ShaderMgr->AddShader("PosNorPS", CShader::ST_PS, L"../bin/Shaders1/Default3.hlsl", nullptr);

	m_ShaderMgr->AddShader("EffectVS", CShader::ST_VS, L"../bin/Shaders/Effect.hlsl", nullptr);
	m_ShaderMgr->AddShader("EffectPS", CShader::ST_PS, L"../bin/Shaders/Effect.hlsl", nullptr);

	m_ShaderMgr->AddShader("UIVS", CShader::ST_VS, L"../bin/Shaders/UI.hlsl", nullptr);
	m_ShaderMgr->AddShader("UIPS", CShader::ST_PS, L"../bin/Shaders/UI.hlsl", nullptr);

	m_ShaderMgr->AddShader("ShadowVS", CShader::ST_VS, L"../bin/Shaders1/Shadows.hlsl", nullptr);
	m_ShaderMgr->AddShader("ShadowPS", CShader::ST_PS, L"../bin/Shaders1/Shadows.hlsl", nullptr);

	m_ShaderMgr->AddShader("BulletPathVS", CShader::ST_VS, L"../bin/Shaders1/BulletInstance.hlsl", nullptr);
	m_ShaderMgr->AddShader("BulletPathPS", CShader::ST_PS, L"../bin/Shaders1/BulletInstance.hlsl", nullptr);

	m_ShaderMgr->AddShader("PostProcessVS", CShader::ST_VS, L"../bin/Shaders1/PostProcess.hlsl", nullptr);
	m_ShaderMgr->AddShader("CopyToBackBufferPS", CShader::ST_PS, L"../bin/Shaders1/PostProcess.hlsl", nullptr);
	// 같은 파일에서 진입점만 다르게: CShader 는 진입점 이름을 "PS" 로 고정하므로 #define 으로 고른다
	{
		static const D3D_SHADER_MACRO brightPassDefines[] = { { "BRIGHT_PASS", "1" }, { nullptr, nullptr } };
		m_ShaderMgr->AddShader("BrightPassPS", CShader::ST_PS, L"../bin/Shaders1/PostProcess.hlsl", brightPassDefines);
		static const D3D_SHADER_MACRO blurDefines[] = { { "BLUR", "1" }, { nullptr, nullptr } };
		m_ShaderMgr->AddShader("BlurPS", CShader::ST_PS, L"../bin/Shaders1/PostProcess.hlsl", blurDefines);
		static const D3D_SHADER_MACRO compositeDefines[] = { { "COMPOSITE", "1" }, { nullptr, nullptr } };
		m_ShaderMgr->AddShader("CompositePS", CShader::ST_PS, L"../bin/Shaders1/PostProcess.hlsl", compositeDefines);
	}
	// Deferred: G-Buffer 기록 PS 는 기존 셰이더 파일에 GBUFFER 를 정의해서 컴파일 (VS 는 기존 DefaultVS / PosNorVS 그대로)
	{
		static const D3D_SHADER_MACRO gbufferDefines[] = { { "GBUFFER", "1" }, { nullptr, nullptr } };
		m_ShaderMgr->AddShader("GBufferPS", CShader::ST_PS, L"../bin/Shaders1/Default.hlsl", gbufferDefines);
		m_ShaderMgr->AddShader("GBufferPosNorPS", CShader::ST_PS, L"../bin/Shaders1/Default3.hlsl", gbufferDefines);
		m_ShaderMgr->AddShader("DeferredLightingVS", CShader::ST_VS, L"../bin/Shaders1/DeferredLighting.hlsl", nullptr);
		m_ShaderMgr->AddShader("DeferredLightingPS", CShader::ST_PS, L"../bin/Shaders1/DeferredLighting.hlsl", nullptr);
	}

	//PSOMgr
	// 렌더타겟 포맷 기준으로 PSO 를 두 묶음으로 나눈다 (PSO 의 RTVFormats 는 실제 바인딩한 RT 포맷과 같아야 한다)
	//   HDR RT (HDR_FORMAT)          : Default, Sky, PosNor, Effect, Ping, BulletPath  → SetRTVFormat(HDR_FORMAT)
	//   백버퍼 (R8G8B8A8_UNORM, 기본) : UI, CopyToBackBuffer
	//   RT 없음                      : Shadow
	m_PSOMgr = CPSOMgr::Get_Instance();

	m_PSOMgr->AddPSO("DefaultPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("DefaultVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("DefaultPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		SetRTVFormat(HDR_FORMAT)->
		Create_PSO());

	m_PSOMgr->AddPSO("SkyPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_POS_NOR_TEX)->
		SetVS(m_ShaderMgr->GetShaderObj("SkyVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("SkyPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		SetForSkyBox()->
		SetRTVFormat(HDR_FORMAT)->
		Create_PSO());

	m_PSOMgr->AddPSO("PosNorPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("PosNorVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("PosNorPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		SetRTVFormat(HDR_FORMAT)->
		Create_PSO());

	m_PSOMgr->AddPSO("EffectPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("EffectVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("EffectPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		set_Disable_Depth_Write()->
		set_Blend_Enable()->
		SetRTVFormat(HDR_FORMAT)->
		Create_PSO());

	m_PSOMgr->AddPSO("UIPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("UIVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("UIPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		set_Blend_Enable()->
		set_Disable_Depth_Write()->
		set_Disable_Depth_Test()->
		Create_PSO());

	m_PSOMgr->AddPSO("PingPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("EffectVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("EffectPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		set_Blend_Enable()->
		set_Disable_Depth_Write()->
		set_Disable_Depth_Test()->
		SetRTVFormat(HDR_FORMAT)->		// RG_BLEND 안에서 Ping 이 ChangePSO 로 바꿔 쓰므로 HDR RT 기준
		Create_PSO());			//그리는 순서는 유아이보다 먼저 그리는 방법은 PosinImage처럼

	m_PSOMgr->AddPSO("ShadowPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("ShadowVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("ShadowPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		setForShadow()->
		Create_PSO());

	m_PSOMgr->AddPSO("BulletPathPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("BulletPathVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("BulletPathPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		set_Blend_Enable()->
		set_Disable_Depth_Write()->
		SetRTVFormat(HDR_FORMAT)->
		Create_PSO());

	// HDR RT → 백버퍼 풀스크린 복사 (톤매핑 전 임시). 정점 버퍼 없음, 깊이 안 씀
	// DSV 포맷은 기본값(D24S8) 그대로: 백버퍼 패스에서 메인 DSV 를 같이 바인딩해 두기 때문
	m_PSOMgr->AddPSO("CopyToBackBufferPSO", CPSO::Create()->
		SetNoInputLayout()->
		SetVS(m_ShaderMgr->GetShaderObj("PostProcessVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("CopyToBackBufferPS"))->
		SetRS(m_RootSignatureMgr->Get("PostProcessRS"))->
		set_Disable_Depth_Test()->
		set_Disable_Depth_Write()->
		set_Cull_None()->
		Create_PSO());

	// HDR + Bloom 합성 + 톤매핑 → 백버퍼. 복사 PSO 와 같은 설정에 PS 만 다르다 (복사 PSO 는 B 키 디버그 보기에 계속 쓴다)
	m_PSOMgr->AddPSO("CompositePSO", CPSO::Create()->
		SetNoInputLayout()->
		SetVS(m_ShaderMgr->GetShaderObj("PostProcessVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("CompositePS"))->
		SetRS(m_RootSignatureMgr->Get("PostProcessRS"))->
		set_Disable_Depth_Test()->
		set_Disable_Depth_Write()->
		set_Cull_None()->
		Create_PSO());

	// Bloom bright pass: HDR RT(전체 해상도) → Bloom A(절반 해상도, HDR_FORMAT). 깊이 버퍼 없음
	m_PSOMgr->AddPSO("BrightPassPSO", CPSO::Create()->
		SetNoInputLayout()->
		SetVS(m_ShaderMgr->GetShaderObj("PostProcessVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("BrightPassPS"))->
		SetRS(m_RootSignatureMgr->Get("PostProcessRS"))->
		SetRTVFormat(HDR_FORMAT)->
		SetNoDepthStencil()->
		set_Cull_None()->
		Create_PSO());

	// Bloom 분리형 가우시안 (가로/세로 공용, 방향은 루트 상수로). Bloom A ↔ B 핑퐁
	m_PSOMgr->AddPSO("BlurPSO", CPSO::Create()->
		SetNoInputLayout()->
		SetVS(m_ShaderMgr->GetShaderObj("PostProcessVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("BlurPS"))->
		SetRS(m_RootSignatureMgr->Get("PostProcessRS"))->
		SetRTVFormat(HDR_FORMAT)->
		SetNoDepthStencil()->
		set_Cull_None()->
		Create_PSO());

	// Deferred G-Buffer 패스: 같은 정점 셰이더, PS 만 G-Buffer 4장(MRT)에 쓰는 버전. 깊이는 평소처럼 쓴다
	{
		const DXGI_FORMAT gbufferFormats[GBUFFER_COUNT] = {
			GBUFFER_FORMAT_ALBEDO, GBUFFER_FORMAT_NORMAL, GBUFFER_FORMAT_MATERIAL, GBUFFER_FORMAT_EMISSIVE };

		m_PSOMgr->AddPSO("GBufferPSO", CPSO::Create()->
			SetInputLayout(CPSO::IT_MESH)->
			SetVS(m_ShaderMgr->GetShaderObj("DefaultVS"))->
			SetPS(m_ShaderMgr->GetShaderObj("GBufferPS"))->
			SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
			SetRTVFormats(GBUFFER_COUNT, gbufferFormats)->
			Create_PSO());

		m_PSOMgr->AddPSO("GBufferPosNorPSO", CPSO::Create()->
			SetInputLayout(CPSO::IT_MESH)->
			SetVS(m_ShaderMgr->GetShaderObj("PosNorVS"))->
			SetPS(m_ShaderMgr->GetShaderObj("GBufferPosNorPS"))->
			SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
			SetRTVFormats(GBUFFER_COUNT, gbufferFormats)->
			Create_PSO());
	}

	// Deferred 라이팅 패스: G-Buffer + 깊이를 읽어 HDR RT 에 조명 결과를 쓴다. 풀스크린 삼각형, 깊이 버퍼 안 묶음
	// (깊이를 SRV 로 읽는 중이라 DSV 로 동시에 묶을 수 없다)
	m_PSOMgr->AddPSO("DeferredLightingPSO", CPSO::Create()->
		SetNoInputLayout()->
		SetVS(m_ShaderMgr->GetShaderObj("DeferredLightingVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("DeferredLightingPS"))->
		SetRS(m_RootSignatureMgr->Get("DeferredLightingRS"))->
		SetRTVFormat(HDR_FORMAT)->
		SetNoDepthStencil()->
		set_Cull_None()->
		Create_PSO());

	//Renderer
	m_MainRenderer = CRenderer::Create(Get_Device(),Get_CommandList(),this);
	m_ComponentMgr->AddPrototype("RendererCom", m_MainRenderer);

	//ObjectMgr
	m_ObjectMgr = CObjectMgr::Get_Instance();

	//MaterialMgr
	m_MaterialMgr = CMaterialMgr::Get_Instance();
	m_MaterialMgr->Resize();

	//TextureMgr
	m_TextureMgr = CTextureMgr::Get_Instance();
	m_TextureMgr->Resize_TexMap();
	m_TextureMgr->Make_DescriptorHeap();
	m_TextureMgr->Add_DefaultTexture();	// 텍스처 인덱스 0 = 분홍. MainApp 텍스처보다 먼저 등록해야 실제 텍스처가 1번부터 된다

	m_ShadowMap = CShadowMap::Get_Instance();
	m_ShadowMap->Initialize();

	m_LightMgr = CLightMgr::Create();

	// HDR 씬 렌더타겟 (Bloom 1단계). 백버퍼와 같은 해상도, RTV/SRV 는 엔진 예약 칸에 만든다
	// 클리어 값 = Clear() 에서 쓰는 색. 메인 패스는 스카이가 먼저 화면 전체를 덮으므로 검정이면 충분
	{
		const FLOAT hdrClearColor[4] = { 0.f, 0.f, 0.f, 1.f };
		m_HDRTarget = CRenderTarget::Create(Get_ClientWidth(), Get_ClientHeight(), HDR_FORMAT, hdrClearColor,
			Get_EngineRtv(RTV_SLOT_HDR),
			Get_EngineSrvCpuHandle(SRV_SLOT_HDR),
			Get_EngineSrvGpuHandle(SRV_SLOT_HDR));

		// Bloom 핑퐁 RT 2장 (절반 해상도). 블러는 넓게 퍼질수록 좋고 픽셀 수가 1/4 이라 싸다
		// A: bright pass 결과 → 세로 블러 결과(최종 bloom), B: 가로 블러 결과(중간)
		const UINT bloomWidth = (Get_ClientWidth() + 1) / 2;
		const UINT bloomHeight = (Get_ClientHeight() + 1) / 2;
		const UINT bloomRtv[2] = { RTV_SLOT_BLOOM_A, RTV_SLOT_BLOOM_B };
		const UINT bloomSrv[2] = { SRV_SLOT_BLOOM_A, SRV_SLOT_BLOOM_B };
		for (int i = 0; i < 2; ++i)
		{
			m_BloomTarget[i] = CRenderTarget::Create(bloomWidth, bloomHeight, HDR_FORMAT, hdrClearColor,
				Get_EngineRtv(bloomRtv[i]),
				Get_EngineSrvCpuHandle(bloomSrv[i]),
				Get_EngineSrvGpuHandle(bloomSrv[i]));
		}
	}

	// Deferred G-Buffer 4장 (전체 해상도). RTV 엔진 3~6, SRV 엔진 4~7
	// 라이팅 패스가 깊이 1(하늘)인 픽셀은 안 읽고, 나머지는 기하 패스가 매 프레임 덮어쓰므로 Clear 는 안 한다
	{
		const DXGI_FORMAT gbufferFormats[GBUFFER_COUNT] = {
			GBUFFER_FORMAT_ALBEDO, GBUFFER_FORMAT_NORMAL, GBUFFER_FORMAT_MATERIAL, GBUFFER_FORMAT_EMISSIVE };
		const FLOAT zeroColor[4] = { 0.f, 0.f, 0.f, 0.f };
		for (int i = 0; i < GBUFFER_COUNT; ++i)
		{
			m_GBuffer[i] = CRenderTarget::Create(Get_ClientWidth(), Get_ClientHeight(), gbufferFormats[i], zeroColor,
				Get_EngineRtv(RTV_SLOT_GBUFFER0 + i),
				Get_EngineSrvCpuHandle(SRV_SLOT_GBUFFER0 + i),
				Get_EngineSrvGpuHandle(SRV_SLOT_GBUFFER0 + i));
		}

		// 메인 깊이 버퍼에 SRV 추가 (그림자맵과 같은 방식: 리소스는 TYPELESS, DSV 는 D24_UNORM_S8_UINT, SRV 는 R24_UNORM_X8_TYPELESS)
		// 깊이 24비트만 읽고 스텐실 8비트는 버린다. 그래픽 장치가 OnResize 로 깊이 버퍼를 다시 만들면 이 SRV 도 다시 만들어야 한다
		D3D12_SHADER_RESOURCE_VIEW_DESC depthSrv{};
		depthSrv.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		depthSrv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		depthSrv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		depthSrv.Texture2D.MostDetailedMip = 0;
		depthSrv.Texture2D.MipLevels = 1;
		Get_Device()->CreateShaderResourceView(Get_DepthStencilBuffer(), &depthSrv, Get_EngineSrvCpuHandle(SRV_SLOT_DEPTH));
	}

	m_Graphic_Dev->Execute_CommandList();

	m_FrameResourceMgr->Flush_CommandQueue();

	m_FrameResourceMgr->Reset_CommandList_and_Allocator(nullptr);

}

void CGameInstance::Update(CTimer* pTimer)
{
	m_FrameResourceMgr->BeginFrame();

	m_ObjectMgr->Update(pTimer->DeltaTime());
}

void CGameInstance::Late_Update(CTimer* pTimer)
{
	m_ShadowMap->Late_Update();

	m_LightMgr->Update(pTimer->DeltaTime());	// 수명 끝난 점광원 제거 (오브젝트 LateTick 에서 새로 넣은 건 이번 프레임에 바로 그려진다)
	
	m_ObjectMgr->LateUpdate(pTimer->DeltaTime());

	m_MaterialMgr->Update_Mats();
}

void CGameInstance::Draw()
{
	m_MainRenderer->Render();

	m_FrameResourceMgr->SignalAndAdvance();

	m_MainRenderer->ResetRenderObjects();
}

void CGameInstance::Free()
{
	//CGameInstance::Release();
}

void CGameInstance::Release_Engine()
{
	m_FrameResourceMgr->Flush_CommandQueue();
	Safe_Release(m_MainRenderer); //Renderer만 삭제할 때, GameInstance를 릴리즈 해야 합니다.
	CGameInstance::Release();
	Safe_Release(m_ObjectMgr);
	Safe_Release(m_MainRenderer);
	Safe_Release(m_ComponentMgr);
	Safe_Release(m_ShaderMgr);
	Safe_Release(m_PSOMgr);
	Safe_Release(m_RootSignatureMgr);
	Safe_Release(m_FrameResourceMgr);
	// D3D 리소스를 가진 매니저는 Graphic_Dev(Device)보다 먼저 해제한다.
	// Graphic_Dev::Free()의 ReportLiveDeviceObjects에 정상 텍스처가 누수로 섞여 나오지 않게 하기 위함
	for (int i = 0; i < GBUFFER_COUNT; ++i)
		Safe_Release(m_GBuffer[i]);
	Safe_Release(m_LightMgr);
	Safe_Release(m_BloomTarget[0]);
	Safe_Release(m_BloomTarget[1]);
	Safe_Release(m_HDRTarget);
	Safe_Release(m_ShadowMap);
	Safe_Release(m_TextureMgr);
	Safe_Release(m_MaterialMgr);
	Safe_Release(m_Graphic_Dev);
	Safe_Release(m_Input_Dev);
	//Safe_Release(m_TextureMgr);	// [수정 전 위치]
	//Safe_Release(m_MaterialMgr);	// [수정 전 위치]
	Safe_Release(m_RandomMgr);
}
