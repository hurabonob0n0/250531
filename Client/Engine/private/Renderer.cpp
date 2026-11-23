#include "Renderer.h"
#include "RenderObject.h"
#include "GameInstance.h"
#include "FrameResourceMgr.h"

static int DebugMode = 0;

// Bloom 디버그/조절 (Engine_Config.h BLOOM_THRESHOLD_DEFAULT)
//   위/아래 화살표 : bright pass 임계값 ±0.05
//   B             : 화면 보기 순환 0 = 최종 씬, 1 = bright pass 결과(블러 안 함), 2 = 블러까지 끝난 Bloom
static float BloomThreshold = BLOOM_THRESHOLD_DEFAULT;
static int   BloomDebugView = 0;

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
    m_CommandList->SetGraphicsRootDescriptorTable(5, m_GameInstance->Get_TextureTableGpuHandle());	// gTextureMaps[0] = 힙 SRV_ENGINE_RESERVED 칸
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
    // [수정 전] 메인 패스를 백버퍼에 바로 그렸다
    //m_GameInstance->Set_BackBuffer_and_DSV();

    // 1) 씬은 HDR RT 에 그린다 (PIXEL_SHADER_RESOURCE → RENDER_TARGET)
    CRenderTarget* pHDR = m_GameInstance->m_HDRTarget;
    pHDR->Transition(m_CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_GameInstance->Set_ViewPort_and_ScissorRect();    // 그림자 패스가 바꿔 둔 뷰포트를 화면 크기로 되돌림
    pHDR->Clear(m_CommandList);
    D3D12_CPU_DESCRIPTOR_HANDLE hdrRtv = pHDR->Get_RTV();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_GameInstance->DepthStencilView();
    m_CommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    m_CommandList->OMSetRenderTargets(1, &hdrRtv, true, &dsv);

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("SkyPSO"));
    m_GameInstance->Set_DescriptorHeap();

    if (m_GameInstance->Key_Down(VK_RIGHT))
        DebugMode += 1;

    if (DebugMode > 2)
        DebugMode = 0;

    if (m_GameInstance->Key_Down('B'))
        BloomDebugView = (BloomDebugView + 1) % 3;
    if (m_GameInstance->Key_Down(VK_UP) || m_GameInstance->Key_Down(VK_DOWN))
    {
        BloomThreshold += m_GameInstance->Key_Down(VK_UP) ? 0.05f : -0.05f;
        if (BloomThreshold < 0.0f)
            BloomThreshold = 0.0f;
        wchar_t msg[64];
        swprintf_s(msg, L"[Bloom] threshold = %.2f\n", BloomThreshold);
        OutputDebugStringW(msg);
    }

    Bind_DefaultRS_for_MainPass();

    Render_Priority();
    
    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("PosNorPSO"));

    Render_NonLight();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("DefaultPSO"));

    Render_NonBlend();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("EffectPSO"));
    Render_Blend();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("BulletPathPSO"));
    Render_BulletPath();

    // 2) HDR RT 를 다 그렸으니 셰이더가 읽을 수 있는 상태로 (RENDER_TARGET → PIXEL_SHADER_RESOURCE)
    pHDR->Transition(m_CommandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // 2-1) Bloom bright pass: HDR(전체 해상도) → Bloom A(절반 해상도), 임계값 넘는 밝기만
    //      PostProcessRS 로 바꾸면 DefaultRS 바인딩은 무효가 된다 (UI 앞에서 다시 건다)
    CRenderTarget* pBloomA = m_GameInstance->m_BloomTarget[0];
    pBloomA->Transition(m_CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    pBloomA->Bind_as_Output(m_CommandList);     // 뷰포트/시저 = 절반 해상도, DSV 없음
    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("BrightPassPSO"));
    m_CommandList->SetGraphicsRootSignature(m_GameInstance->GetRootSignature("PostProcessRS"));
    m_CommandList->SetGraphicsRootDescriptorTable(0, pHDR->Get_SRV());
    const float brightParams[4] = { BloomThreshold, 0.f, 0.f, 0.f };
    m_CommandList->SetGraphicsRoot32BitConstants(1, 4, brightParams, 0);
    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_CommandList->DrawInstanced(3, 1, 0, 0);    // 화면 전체를 덮으므로 Clear 불필요
    pBloomA->Transition(m_CommandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // 2-2) 분리형 가우시안 핑퐁: 가로 A → B, 세로 B → A. 최종 Bloom 은 A 에 남는다
    //      읽는 RT 는 PIXEL_SHADER_RESOURCE, 쓰는 RT 는 RENDER_TARGET. 같은 RT 를 읽으면서 쓸 수는 없어서 2장이 필요
    //      B 키 보기 1(bright pass 결과)에서는 블러를 건너뛴다
    CRenderTarget* pBloomB = m_GameInstance->m_BloomTarget[1];
    if (BloomDebugView != 1)
    {
        m_CommandList->SetPipelineState(m_GameInstance->GetPSO("BlurPSO"));   // RS 는 PostProcessRS 그대로
        const float texelW = 1.0f / static_cast<float>(pBloomA->Get_Width());
        const float texelH = 1.0f / static_cast<float>(pBloomA->Get_Height());

        for (int i = 0; i < BLOOM_BLUR_ITERATIONS; ++i)
        {
            // 가로: A 읽기 → B 쓰기
            pBloomB->Transition(m_CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
            pBloomB->Bind_as_Output(m_CommandList);
            m_CommandList->SetGraphicsRootDescriptorTable(0, pBloomA->Get_SRV());
            const float horizontal[4] = { texelW, 0.f, 0.f, 0.f };
            m_CommandList->SetGraphicsRoot32BitConstants(1, 4, horizontal, 0);
            m_CommandList->DrawInstanced(3, 1, 0, 0);
            pBloomB->Transition(m_CommandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            // 세로: B 읽기 → A 쓰기
            pBloomA->Transition(m_CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
            pBloomA->Bind_as_Output(m_CommandList);
            m_CommandList->SetGraphicsRootDescriptorTable(0, pBloomB->Get_SRV());
            const float vertical[4] = { 0.f, texelH, 0.f, 0.f };
            m_CommandList->SetGraphicsRoot32BitConstants(1, 4, vertical, 0);
            m_CommandList->DrawInstanced(3, 1, 0, 0);
            pBloomA->Transition(m_CommandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }
    }

    // 3) 백버퍼로 전환 (PRESENT → RENDER_TARGET, 클리어, 백버퍼 + 메인 DSV 바인딩)
    //    복사 패스가 화면 전체를 덮으므로 백버퍼 클리어는 사실상 필요 없지만 기존 함수를 그대로 쓴다
    m_GameInstance->Set_BackBuffer_and_DSV();

    // 4) HDR → 백버퍼 풀스크린 복사 (임시. 다음 단계에서 Bloom 합성 + 톤매핑으로 바뀐다)
    //    HDR(R16G16B16A16_FLOAT)과 백버퍼(R8G8B8A8_UNORM)는 포맷이 달라 CopyResource 를 못 쓴다 → 셰이더로 그린다
    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("CopyToBackBufferPSO"));
    m_CommandList->SetGraphicsRootSignature(m_GameInstance->GetRootSignature("PostProcessRS"));
    // B 키 디버그 보기: HDR 대신 Bloom A 를 화면 크기로 늘려 그린다 (Set_BackBuffer_and_DSV 가 뷰포트를 화면 크기로 되돌려 둠)
    m_CommandList->SetGraphicsRootDescriptorTable(0, BloomDebugView != 0 ? pBloomA->Get_SRV() : pHDR->Get_SRV());
    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_CommandList->DrawInstanced(3, 1, 0, 0);

    // 5) UI 는 톤매핑 영향을 받지 않게 백버퍼에 직접 그린다
    //    루트시그니처를 바꾸면 이전 바인딩이 전부 무효가 되므로 DefaultRS 를 다시 바인딩한다
    Bind_DefaultRS_for_MainPass();
    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("UIPSO"));
    Render_UI();
    
    m_GameInstance->Present();

    PassNum = 2;

#pragma endregion Normal Pass
}

// 메인 패스(씬, UI)가 쓰는 DefaultRS 와 패스 공통 바인딩
// SetGraphicsRootSignature 를 다시 부르면 루트 인자가 전부 초기화되므로, RS 를 바꿨다 돌아올 때마다 같이 다시 건다
void CRenderer::Bind_DefaultRS_for_MainPass()
{
    m_CommandList->SetGraphicsRootSignature(m_GameInstance->GetRootSignature("DefaultRS"));
    m_GameInstance->Set_CurrentFramePBMats(0);                                                      // slot 1 PassCB(메인 카메라), slot 2 머티리얼
    m_CommandList->SetGraphicsRootDescriptorTable(4, m_GameInstance->m_ShadowMap->mhGpuSrv);        // slot 4 그림자맵
    m_CommandList->SetGraphicsRootDescriptorTable(5, m_GameInstance->Get_TextureTableGpuHandle());  // slot 5 gTextureMaps[0] = 힙 SRV_ENGINE_RESERVED 칸
    m_CommandList->SetGraphicsRoot32BitConstant(7, DebugMode, 0);                                    // slot 7 디버그 모드
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
