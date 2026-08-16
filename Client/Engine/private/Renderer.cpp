#include "Renderer.h"
#include "RenderObject.h"
#include "GameInstance.h"
#include "FrameResourceMgr.h"

CRenderer::CRenderer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, class CGameInstance* pGameInstance) : CComponent(pDevice, pCommandList)
{
    m_GameInstance = pGameInstance;
    Safe_AddRef(m_GameInstance);
}

CRenderer::CRenderer(CRenderer& rhs) : CComponent(rhs)
{
}

void CRenderer::AddtoRenderObjects(RENDERGROUP RG, CRenderObject* pRenderObject)
{
    Safe_AddRef(pRenderObject);
    m_vRenderObjects[RG].push_back(pRenderObject);
}

void CRenderer::ResetRenderObjects()
{
    for (auto& pRenderObject : m_vRenderObjects[RG_PRIORITY]) {
        Safe_Release(pRenderObject);
    }
    m_vRenderObjects[RG_PRIORITY].clear();

    for (auto& pRenderObject : m_vRenderObjects[RG_NONLIGHT]) {
        Safe_Release(pRenderObject);
    }
    m_vRenderObjects[RG_NONLIGHT].clear();


    for (auto& pRenderObject : m_vRenderObjects[RG_NONBLEND]) {
        Safe_Release(pRenderObject);
    }
    m_vRenderObjects[RG_NONBLEND].clear();

    for (auto& pRenderObject : m_vRenderObjects[RG_BLEND]) {
        Safe_Release(pRenderObject);
    }
    m_vRenderObjects[RG_BLEND].clear();

    for (auto& pRenderObject : m_vRenderObjects[RG_UI]) {
        Safe_Release(pRenderObject);
    }
    m_vRenderObjects[RG_UI].clear();

    for (auto& pRenderObject : m_vRenderObjects[RG_BULLETPATH]) {
        Safe_Release(pRenderObject);
    }
    m_vRenderObjects[RG_BULLETPATH].clear();
}

void CRenderer::ChangePSO(const string& PSOName)
{
    m_CommandList->SetPipelineState(m_GameInstance->GetPSO(PSOName));
}

void CRenderer::Resize_RenderGroups()
{
    for (int i = 0; i < (int)RG_BULLETPATH; ++i)
    {
        m_vRenderObjects[i].reserve(1000);
    }
}

HRESULT CRenderer::Initialize_Prototype()
{
    __super::Initialize_Prototype();
    Resize_RenderGroups();
    return S_OK;
}

HRESULT CRenderer::Initialize(void* pArg)
{
    __super::Initialize(pArg);
    return S_OK;
}

void CRenderer::Render()
{
#pragma region ShadowPass
    m_GameInstance->Reset_CommandList_and_Allocator(m_GameInstance->GetPSO("ShadowPSO"));
    m_GameInstance->Set_DescriptorHeap();
    m_CommandList->SetGraphicsRootSignature(m_GameInstance->GetRootSignature("DefaultRS"));
    m_GameInstance->Set_CurrentFramePBMats(1);
    m_CommandList->SetGraphicsRootDescriptorTable(5, m_GameInstance->Get_SRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
    //m_GameInstance->Set_ViewPort_and_ScissorRect();
    m_CommandList->RSSetViewports(1, &m_GameInstance->m_ShadowMap->mViewport);
    m_CommandList->RSSetScissorRects(1, &m_GameInstance->m_ShadowMap->mScissorRect);
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_GameInstance->m_ShadowMap->m_Resource,
        D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
    m_CommandList->ClearDepthStencilView(m_GameInstance->m_ShadowMap->mhCpuDsv,
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    m_CommandList->OMSetRenderTargets(0, nullptr, false, &m_GameInstance->m_ShadowMap->mhCpuDsv);
    m_GameInstance->Set_ShadowPass(true);
    Render_NonBlend();
    m_GameInstance->Set_ShadowPass(false);
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_GameInstance->m_ShadowMap->m_Resource,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
#pragma endregion ShadowPass

    --PassNum;

#pragma region Normal Pass
    //m_GameInstance->Reset_CommandList_and_Allocator(m_GameInstance->GetPSO("SkyPSO"));
    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("SkyPSO"));
    m_GameInstance->Set_BackBuffer_and_DSV();
    m_GameInstance->Set_DescriptorHeap();
    m_CommandList->SetGraphicsRootSignature(m_GameInstance->GetRootSignature("DefaultRS"));
    m_GameInstance->Set_CurrentFramePBMats(0);
    m_CommandList->SetGraphicsRootDescriptorTable(4, m_GameInstance->m_ShadowMap->mhGpuSrv);
    m_CommandList->SetGraphicsRootDescriptorTable(5, m_GameInstance->Get_SRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

    Render_Priority();
    
    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("PosNorPSO"));

    Render_NonLight();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("DefaultPSO"));

    Render_NonBlend();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("EffectPSO"));
    Render_Blend();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("BulletPathPSO"));
    Render_BulletPath();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("UIPSO"));
    Render_UI();
    
    m_GameInstance->Present();

    PassNum = 2;

#pragma endregion Normal Pass
}

void CRenderer::Render_Priority()
{
    for (auto& pGameObject : m_vRenderObjects[RG_PRIORITY])
    {
        pGameObject->Render();

        Safe_Release(pGameObject);
    }
    m_vRenderObjects[RG_PRIORITY].clear();
}

void CRenderer::Render_NonLight()
{
    for (auto& pGameObject : m_vRenderObjects[RG_NONLIGHT])
    {
        pGameObject->Render();

        Safe_Release(pGameObject);
    }

    m_vRenderObjects[RG_NONLIGHT].clear();
}

void CRenderer::Render_NonBlend()
{
    for (auto& pGameObject : m_vRenderObjects[RG_NONBLEND])
    {
        pGameObject->Render();
        //Safe_Release(pGameObject);
        if (PassNum == 1)
        {
            Safe_Release(pGameObject);
        }
    }
    if(PassNum == 1)
        m_vRenderObjects[RG_NONBLEND].clear();
}

void CRenderer::Render_Blend()
{
    for (auto& pGameObject : m_vRenderObjects[RG_BLEND])
    {
        pGameObject->Render();

        Safe_Release(pGameObject);
    }
    m_vRenderObjects[RG_BLEND].clear();
}

void CRenderer::Render_UI()
{
    for (auto& pGameObject : m_vRenderObjects[RG_UI])
    {
        pGameObject->Render();

        Safe_Release(pGameObject);
    }
    m_vRenderObjects[RG_UI].clear();
}

void CRenderer::Render_BulletPath()
{
    for (auto& pGameObject : m_vRenderObjects[RG_BULLETPATH])
    {
        pGameObject->Render();

        Safe_Release(pGameObject);
    }
    m_vRenderObjects[RG_BULLETPATH].clear();
}

CRenderer* CRenderer::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, CGameInstance* pInstance)
{
    CRenderer* pRenderer = new CRenderer(pDevice, pCommandList,pInstance);

    if (FAILED(pRenderer->Initialize_Prototype()))
    {
        MSG_BOX("Failed to create Renderer prototype.");
        Safe_Release(pRenderer);
    }

    return pRenderer;
}

CComponent* CRenderer::Clone(void* pArg)
{
    AddRef();
    return this;
}

void CRenderer::Free()
{
    ResetRenderObjects();
    __super::Free();
}
