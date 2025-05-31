#pragma once
#include "Component.h"

BEGIN(Engine)

class CRenderObject;

class ENGINE_DLL CRenderer : public CComponent
{
public:
	enum RENDERGROUP { RG_PRIORITY, RG_NONLIGHT, RG_NONBLEND, RG_BLEND, RG_UI, RG_END };

public:
	CRenderer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList ,class CGameInstance* pGameInstance);
	CRenderer(CRenderer& rhs);
	virtual ~CRenderer() = default;

public:
	void AddtoRenderObjects(RENDERGROUP RG, CRenderObject* pRenderObject);
	void ResetRenderObjects();

public:
	void Resize_RenderGroups();

public:
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;
	void Render();

private:
	vector<CRenderObject*> m_vRenderObjects[RG_END];

private:
	class CGameInstance* m_GameInstance;

private:
	void Render_Priority();
	void Render_NonLight();
	void Render_NonBlend();
	void Render_Blend();
	void Render_UI();

private:
	//Todo : 나중에 PSO, RootSignature, FrameResource 등등 렌더링에 필요한 것들을 모두 모아놓고 Render()함수 안에서 처리하도록 만든다.

public:
	static CRenderer* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList,class CGameInstance* pInstance);
	CComponent* Clone(void* pArg) override;
	void Free() override;
};

END