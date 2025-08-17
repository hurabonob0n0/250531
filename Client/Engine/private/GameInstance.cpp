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

	//PSOMgr
	m_PSOMgr = CPSOMgr::Get_Instance();

	m_PSOMgr->AddPSO("DefaultPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("DefaultVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("DefaultPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->Create_PSO());

	m_PSOMgr->AddPSO("SkyPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_POS_NOR_TEX)->
		SetVS(m_ShaderMgr->GetShaderObj("SkyVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("SkyPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		SetForSkyBox()->Create_PSO());

	m_PSOMgr->AddPSO("PosNorPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("PosNorVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("PosNorPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		Create_PSO());

	m_PSOMgr->AddPSO("EffectPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("EffectVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("EffectPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		set_Disable_Depth_Write()-> 
		set_Blend_Enable()->
		Create_PSO());

	m_PSOMgr->AddPSO("UIPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("UIVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("UIPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		set_Blend_Enable()->
		set_Disable_Depth_Write()->
		//set_Disable_Depth_Test()->
		Create_PSO());

	m_PSOMgr->AddPSO("PingPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_MESH)->
		SetVS(m_ShaderMgr->GetShaderObj("EffectVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("EffectPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->
		set_Blend_Enable()->
		set_Disable_Depth_Write()->
		set_Disable_Depth_Test()->
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
	
	m_ShadowMap = CShadowMap::Get_Instance();
	m_ShadowMap->Initialize();
	
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
	Safe_Release(m_Graphic_Dev);
	Safe_Release(m_Input_Dev);
	Safe_Release(m_TextureMgr);
	Safe_Release(m_MaterialMgr);
	Safe_Release(m_RandomMgr);
}
