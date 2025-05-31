#include "GameInstance.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance() : m_Graphic_Dev(CGraphic_Dev::Get_Instance()),m_FrameResourceMgr(CFrameResourceMgr::Get_Instance())
{
}

void CGameInstance::Initialize(WindowInfo windowInfo, CRawInput* pRawInput)
{
	ID3D12Debug* debugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	debugController->EnableDebugLayer();

	//InputDev
	m_Input_Dev = pRawInput;
	Safe_AddRef(m_Input_Dev);

	//GraphicDev
	m_Graphic_Dev->Initialize(windowInfo);
	
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
	m_ShaderMgr->AddShader("DefaultVS", CShader::ST_VS, L"../bin/Shaders/Default.hlsl", nullptr);
	m_ShaderMgr->AddShader("DefaultPS", CShader::ST_PS, L"../bin/Shaders/Default.hlsl", nullptr);

	m_ShaderMgr->AddShader("SkyVS", CShader::ST_VS, L"../bin/Shaders/Sky.hlsl", nullptr);
	m_ShaderMgr->AddShader("SkyPS", CShader::ST_PS, L"../bin/Shaders/Sky.hlsl", nullptr);

	m_ShaderMgr->AddShader("PosNorVS", CShader::ST_VS, L"../bin/ShaderFiles/Default.hlsl", nullptr);
	m_ShaderMgr->AddShader("PosNorPS", CShader::ST_PS, L"../bin/ShaderFiles/Default.hlsl", nullptr);

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

	m_PSOMgr->AddPSO("TerrainPSO", CPSO::Create()->
		SetInputLayout(CPSO::IT_POS_NOR)->
		SetVS(m_ShaderMgr->GetShaderObj("PosNorVS"))->
		SetPS(m_ShaderMgr->GetShaderObj("PosNorPS"))->
		SetRS(m_RootSignatureMgr->Get("DefaultRS"))->Create_PSO());

	//m_PSOMgr->AddPSO("PosNorPSO", CPSO::Create()->
	//	SetInputLayout(CPSO::IT_POS_NOR)->
	//	SetVS(m_ShaderMgr->GetShaderObj("PosNorVS"))->
	//	SetPS(m_ShaderMgr->GetShaderObj("PosNorPS"))->
	//	SetRS(m_RootSignatureMgr->Get("DefaultRS"))->Create_PSO());

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
}
