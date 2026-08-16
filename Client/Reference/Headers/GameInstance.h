#pragma once
#include "Graphic_Dev.h"
#include "Timer.h"
#include "RawInput.h"
#include "FrameResourceMgr.h"
#include "ComponentMgr.h"
#include "RootSignatureMgr.h"
#include "Shader_Mgr.h"
#include "PSOMgr.h"
#include "Renderer.h"
#include "ObjectMgr.h"
#include "TextureMgr.h"
#include "MaterialMgr.h"
#include "Random_Manager.h"
#include "ShadowMap.h"

BEGIN(Engine)

class ENGINE_DLL CGameInstance : public CBase
{
	DECLARE_SINGLETON(CGameInstance);
private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public:
	void Initialize(WindowInfo& windowInfo,CRawInput* pRawInput);
	void Update(CTimer* pTimer);
	void Late_Update(CTimer* pTimer);
	void Draw();

public:		//For_GraphicDev
	ID3D12GraphicsCommandList* Get_CommandList() { return m_Graphic_Dev->Get_CommandList(); }
	ID3D12Device* Get_Device() { return m_Graphic_Dev->Get_Device(); }
	ID3D12CommandQueue* Get_CommandQueue() { return m_Graphic_Dev->Get_CommandQueue(); }
	void Execute_CommandList() { m_Graphic_Dev->Execute_CommandList(); }
	UINT Get_CBVUAVSRVHeapSize() { return m_Graphic_Dev->Get_CBVUAVSRVHeapSize(); }
	void Set_BackBuffer_and_DSV() { m_Graphic_Dev->Set_BackBuffer_and_DSV(); }
	void Present() { m_Graphic_Dev->Present(); }
	void OnResize() { m_Graphic_Dev->OnResize(); }
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const { return m_Graphic_Dev->DepthStencilView(); }
	void Set_ViewPort_and_ScissorRect() { m_Graphic_Dev->Set_ViewPort_and_ScissorRect(); }
	UINT						Get_DSVSize() { return m_Graphic_Dev->Get_DSVSize(); }

public:		//For_InputDev
	bool Key_Down(UINT vkey) {return m_Input_Dev->Key_Down(vkey);}
	bool Key_Up(UINT vkey) { return m_Input_Dev->Key_Up(vkey); }
	bool Key_Pressing(UINT vkey) { return m_Input_Dev->Key_Pressing(vkey); }

	//bool Mouse_Down(MOUSEKEYSTATE button);
	//bool Mouse_Up(MOUSEKEYSTATE button);
	//bool Mouse_Pressing(MOUSEKEYSTATE button);
	bool Mouse_Down(UINT button) { return m_Input_Dev->Mouse_Down(button); }
	bool Mouse_Up(UINT button) { return m_Input_Dev->Mouse_Up(button); }

	SHORT Get_Mouse_XDelta() { return m_Input_Dev->Get_Mouse_XDelta(); }
	SHORT Get_Mouse_YDelta() { return m_Input_Dev->Get_Mouse_YDelta(); }
	SHORT Get_Mouse_Scroll() { return m_Input_Dev->Get_Mouse_Scroll(); }

public:		//For_FrameResourceMgr
	void Reset_CommandList_and_Allocator(ID3D12PipelineState* PSO) { m_FrameResourceMgr->Reset_CommandList_and_Allocator(PSO); } //Todo : 나중에 매개변수로 PSO받아서 설정해주기
	CFrameResource* Get_Current_FrameResource() { return m_FrameResourceMgr->Get_Current_FrameResource(); }
	void Flush_CommandQueue() { m_FrameResourceMgr->Flush_CommandQueue(); }
	void Set_CurrentFramePBMats(_uint index) { m_FrameResourceMgr->Set_CurrentFramePBMats(index); }
	void Execute_Flush_and_Reset() { 
		m_Graphic_Dev->Execute_CommandList();
		m_FrameResourceMgr->Flush_CommandQueue();
		m_FrameResourceMgr->Reset_CommandList_and_Allocator(nullptr);
	}
public: //For ComponentMgr
	HRESULT AddPrototype(const string& prototypeTag, CComponent* pPrototype) { return m_ComponentMgr->AddPrototype(prototypeTag, pPrototype); }
	CComponent* Get_Component(const string& prototypeTag, void* pArg = nullptr) const { return m_ComponentMgr->GetComponent(prototypeTag, pArg); }
	CComponent* GetPrototype(const string& prototypeTag) const { return m_ComponentMgr->GetPrototype(prototypeTag); }

public: //For ObjectMgr
	HRESULT AddObject(string PrototypeTag, string layerTag, void* pArg) { return m_ObjectMgr->AddObject(PrototypeTag, layerTag, pArg); }
	HRESULT Add_PrototypeObject(string Prototypename, CGameObject* pGameObject) { return m_ObjectMgr->Add_PrototypeObject(Prototypename, pGameObject); }
	CComponent* Get_Object_Component(string LayerName, _uint ID, string ComponentTag){ return m_ObjectMgr->Get_Object_Component(LayerName, ID, ComponentTag); }

public:
	void Set_Pos_For_Server(string LayerName, _uint ID, float* pos) {
		m_ObjectMgr->Set_Pos_For_Server(LayerName, ID, pos);
	}
	CGameObject* GetGameObject(string LayerName, _uint ID) {

		return m_ObjectMgr->GetGameObject(LayerName, ID);
	}

	int GetLayerSize(string LayerName) {

		return m_ObjectMgr->GetLayerSize(LayerName);
	}

public: //For MaterialMgr
	int Add_Material(string matName, MaterialData matInstance) { return m_MaterialMgr->Add_Material(matName, matInstance); }
	_uint Get_Mat_Size() { return m_MaterialMgr->Get_Mat_Size(); }

public: //For TextureMgr
	int Add_Texture(string texname, CTexture* texInstance, CTextureMgr::TEXTURETYPE TT = CTextureMgr::TT_TEXTURE2D) {
		return m_TextureMgr->Add_Texture(texname, texInstance, TT);
	}
	ID3D12DescriptorHeap* Get_SRVDescriptorHeap() { return m_TextureMgr->Get_DescriptorHeap(); }
	void Set_DescriptorHeap() { m_TextureMgr->Set_DescriptorHeap(); }
	void Add_ShadowMap(ID3D12Resource** Resource) { m_TextureMgr->Add_ShadowMap(Resource); }
	CD3DX12_CPU_DESCRIPTOR_HANDLE Get_CPUSRVDescriptorHeap() { return m_TextureMgr->Get_CPUSRVDescriptorHeap(); }

public: //For PSOMgr
	ID3D12PipelineState* GetPSO(const string& PSOName) const { return m_PSOMgr->Get(PSOName); }

public: //For RSMgr
	ID3D12RootSignature* GetRootSignature(const std::string& name) const { return m_RootSignatureMgr->Get(name); }

public: //For RandomMgr
	void Tick() { m_RandomMgr->Tick(); }
	_int Get_RandomI(_int iStart, _int iEnd)
	{
		return m_RandomMgr->Get_RandomI(iStart, iEnd);
	}
	float Get_RandomF(float Start, float End) {
		return m_RandomMgr->Get_RandomF(Start, End);
	}

public: //For Terrain culling / LOD
	/* Cameras hand over the view-projection they just wrote into the pass CB,
	   so the terrain can cull chunks and pick a LOD without knowing the camera. */
	void Set_CameraViewProj(_fmatrix ViewProj, _fvector EyePos) {
		XMStoreFloat4x4(&m_CameraViewProj, ViewProj);
		XMStoreFloat3(&m_CameraEye, EyePos);
		m_isCameraValid = true;
	}
	void Set_ShadowViewProj(_fmatrix ViewProj) { XMStoreFloat4x4(&m_ShadowViewProj, ViewProj); }

	_matrix		Get_CameraViewProj() const { return XMLoadFloat4x4(&m_CameraViewProj); }
	_matrix		Get_ShadowViewProj() const { return XMLoadFloat4x4(&m_ShadowViewProj); }
	_float3		Get_CameraEye() const { return m_CameraEye; }
	bool		Is_CameraValid() const { return m_isCameraValid; }

	/* CRenderer turns this on while it records the shadow pass. */
	void		Set_ShadowPass(bool isShadowPass) { m_isShadowPass = isShadowPass; }
	bool		Is_ShadowPass() const { return m_isShadowPass; }

private:
	_float4x4	m_CameraViewProj = {};
	_float4x4	m_ShadowViewProj = {};
	_float3		m_CameraEye = {};
	bool		m_isCameraValid = false;
	bool		m_isShadowPass = false;

public:
	void Free();

public:
	void Release_Engine();


private:
	CGraphic_Dev*			m_Graphic_Dev;
	CRawInput*				m_Input_Dev;
	CFrameResourceMgr*		m_FrameResourceMgr;
	CComponentMgr*			m_ComponentMgr;
	CRootSignatureMgr*		m_RootSignatureMgr;
	CShader_Mgr*			m_ShaderMgr;
	CPSOMgr*				m_PSOMgr;
	CRenderer*				m_MainRenderer;
	CObjectMgr*				m_ObjectMgr;
	CMaterialMgr*			m_MaterialMgr;
	CTextureMgr*			m_TextureMgr;
	CRandom_Manager*		m_RandomMgr;

public:
	CShadowMap*				m_ShadowMap;
};

END