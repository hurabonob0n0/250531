#include "Renderer.h"
#include "RenderObject.h"
#include "GameInstance.h"
#include "FrameResourceMgr.h"

static int DebugMode = 0;

// Bloom 디버그/조절 (Engine_Config.h BLOOM_THRESHOLD_DEFAULT)
//   위/아래 화살표 : bright pass 임계값 ±0.05
//   B             : 화면 보기 순환 0 = 최종 씬, 1 = bright pass 결과(블러 안 함), 2 = 블러까지 끝난 Bloom
//   PageUp/PageDown : Bloom 강도 ±0.1 (0 이면 Bloom 끔 → on/off 비교 스크린샷용)
//   N             : 톤매핑 모드 순환 0 없음(saturate), 1 Reinhard, 2 ACES
static float BloomThreshold = BLOOM_THRESHOLD_DEFAULT;
static int   BloomDebugView = 0;
static float BloomIntensity = BLOOM_INTENSITY_DEFAULT;
static int   TonemapMode    = TONEMAP_MODE_DEFAULT;

// Forward / Deferred 전환 (L 키). Engine_Config.h DEFERRED_DEFAULT
static bool  UseDeferred    = (DEFERRED_DEFAULT != 0);

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

    Handle_DebugKeys();
    m_GameInstance->Set_DescriptorHeap();

    // 1) 씬을 HDR RT 에 그린다. 끝나면 HDR RT 는 RENDER_TARGET 상태
    //    L 키로 Forward / Deferred 전환 (같은 화면이 나와야 정상. 조명 수별 비교 측정용)
    CRenderTarget* pHDR = m_GameInstance->m_HDRTarget;
    if (UseDeferred)
        Render_Scene_Deferred();
    else
        Render_Scene_Forward();

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

    // 4) 최종 합성: HDR + Bloom → 노출 → 톤매핑 → 백버퍼 (R8G8B8A8_UNORM, 0~1)
    //    HDR(R16G16B16A16_FLOAT)과 백버퍼는 포맷이 달라 CopyResource 를 못 쓴다 → 셰이더로 그린다
    //    Set_BackBuffer_and_DSV 가 뷰포트를 화면 크기로 되돌려 둔 상태
    m_CommandList->SetGraphicsRootSignature(m_GameInstance->GetRootSignature("PostProcessRS"));
    m_CommandList->SetGraphicsRootDescriptorTable(0, pHDR->Get_SRV());       // t0 씬
    m_CommandList->SetGraphicsRootDescriptorTable(2, pBloomA->Get_SRV());    // t1 Bloom (절반 해상도, 선형 샘플로 늘림)
    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (BloomDebugView == 0)
    {
        m_CommandList->SetPipelineState(m_GameInstance->GetPSO("CompositePSO"));
        const float compositeParams[4] = { BloomIntensity, TONEMAP_EXPOSURE, static_cast<float>(TonemapMode), 0.f };
        m_CommandList->SetGraphicsRoot32BitConstants(1, 4, compositeParams, 0);
    }
    else
    {
        // B 키 디버그 보기: Bloom A 를 톤매핑 없이 그대로 화면 크기로 늘려 그린다
        m_CommandList->SetPipelineState(m_GameInstance->GetPSO("CopyToBackBufferPSO"));
        m_CommandList->SetGraphicsRootDescriptorTable(0, pBloomA->Get_SRV());
    }
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

// 디버그/조절 키. 프레임마다 Render() 시작에서 한 번
void CRenderer::Handle_DebugKeys()
{
    // L : Forward ↔ Deferred
    if (m_GameInstance->Key_Down('L'))
    {
        UseDeferred = !UseDeferred;
        OutputDebugStringW(UseDeferred ? L"[Render] Deferred\n" : L"[Render] Forward\n");
    }

    // 오른쪽 화살표 : 디버그 보기 순환
    //   Forward  0 조명, 1 노멀, 2 그림자
    //   Deferred 0 조명, 1 노멀, 2 그림자, 3 알베도, 4 광택(shininess), 5 발광, 6 깊이  (G-Buffer 시각화)
    const int maxDebugMode = UseDeferred ? 6 : 2;
    if (m_GameInstance->Key_Down(VK_RIGHT))
    {
        DebugMode += 1;
        if (DebugMode > maxDebugMode)
            DebugMode = 0;
        static const wchar_t* names[7] = { L"lit", L"normal", L"shadow", L"albedo", L"shininess", L"emissive", L"depth" };
        wchar_t msg[64];
        swprintf_s(msg, L"[Debug] view %d = %s\n", DebugMode, names[DebugMode]);
        OutputDebugStringW(msg);
    }
    if (DebugMode > maxDebugMode)   // Deferred 에서 3 이상 보다가 Forward 로 바꾼 경우
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
    if (m_GameInstance->Key_Down(VK_PRIOR) || m_GameInstance->Key_Down(VK_NEXT))
    {
        BloomIntensity += m_GameInstance->Key_Down(VK_PRIOR) ? 0.1f : -0.1f;
        if (BloomIntensity < 0.0f)
            BloomIntensity = 0.0f;
        wchar_t msg[64];
        swprintf_s(msg, L"[Bloom] intensity = %.1f\n", BloomIntensity);
        OutputDebugStringW(msg);
    }
    if (m_GameInstance->Key_Down('N'))
    {
        TonemapMode = (TonemapMode + 1) % 3;
        static const wchar_t* names[3] = { L"none (saturate)", L"Reinhard", L"ACES" };
        wchar_t msg[64];
        swprintf_s(msg, L"[Tonemap] %s\n", names[TonemapMode]);
        OutputDebugStringW(msg);
    }
}

// [Forward] 물체를 그리면서 조명까지 계산한다 (Default.hlsl PS 가 그림자, 노멀맵, 조명, 반사를 한 번에)
void CRenderer::Render_Scene_Forward()
{
    CRenderTarget* pHDR = m_GameInstance->m_HDRTarget;
    pHDR->Transition(m_CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_GameInstance->Set_ViewPort_and_ScissorRect();    // 그림자 패스가 바꿔 둔 뷰포트를 화면 크기로 되돌림
    pHDR->Clear(m_CommandList);
    D3D12_CPU_DESCRIPTOR_HANDLE hdrRtv = pHDR->Get_RTV();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_GameInstance->DepthStencilView();
    m_CommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    m_CommandList->OMSetRenderTargets(1, &hdrRtv, true, &dsv);

    Bind_DefaultRS_for_MainPass();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("SkyPSO"));
    Render_Priority();      // 카메라가 스카이를 그리면서 slot 3 에 큐브맵을 건다 (DefaultPS 반사가 이걸 쓴다)

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("PosNorPSO"));
    Render_NonLight();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("DefaultPSO"));
    Render_NonBlend();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("EffectPSO"));
    Render_Blend();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("BulletPathPSO"));
    Render_BulletPath();
}

// [Deferred]
//   (1) 기하 패스  : 불투명 물체 → G-Buffer 4장 + 깊이. 조명 계산 없음
//   (2) 라이팅 패스: 풀스크린 삼각형. 픽셀마다 G-Buffer 를 읽고 깊이로 월드 위치를 복원해 조명 → HDR RT
//   (3) Forward    : 스카이, 반투명(Effect/Ping/BulletPath)을 HDR RT 위에 그대로 그린다
//                    반투명은 G-Buffer 에 담을 수 없다 (픽셀당 표면 하나뿐이라)
void CRenderer::Render_Scene_Deferred()
{
    CRenderTarget* pHDR = m_GameInstance->m_HDRTarget;
    CRenderTarget** ppGBuffer = m_GameInstance->m_GBuffer;
    ID3D12Resource* pDepth = m_GameInstance->Get_DepthStencilBuffer();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_GameInstance->DepthStencilView();

    // (1) 기하 패스 -------------------------------------------------------------
    // G-Buffer 는 Clear 하지 않는다: 하늘(깊이 1) 픽셀은 라이팅 패스가 안 읽고, 나머지는 여기서 전부 덮어쓴다
    D3D12_CPU_DESCRIPTOR_HANDLE gbufferRtv[GBUFFER_COUNT];
    for (int i = 0; i < GBUFFER_COUNT; ++i)
    {
        ppGBuffer[i]->Transition(m_CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        gbufferRtv[i] = ppGBuffer[i]->Get_RTV();
    }
    m_GameInstance->Set_ViewPort_and_ScissorRect();
    m_CommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    // RTV 4개 = PS 의 SV_Target0~3. 칸이 연속이 아니어도 되게 배열로 넘긴다 (RTsSingleHandleToDescriptorRange = FALSE)
    m_CommandList->OMSetRenderTargets(GBUFFER_COUNT, gbufferRtv, FALSE, &dsv);

    Bind_DefaultRS_for_MainPass();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("GBufferPosNorPSO"));
    Render_NonLight();

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("GBufferPSO"));
    Render_NonBlend();

    // (2) 라이팅 패스 -----------------------------------------------------------
    // G-Buffer 와 깊이를 셰이더가 읽을 수 있게. 깊이는 DSV 로 묶은 채로 SRV 로 읽을 수 없어서 출력은 HDR RT 하나만 묶는다
    for (int i = 0; i < GBUFFER_COUNT; ++i)
        ppGBuffer[i]->Transition(m_CommandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDepth,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    pHDR->Transition(m_CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    pHDR->Bind_as_Output(m_CommandList);    // 화면 전체를 덮어쓰므로 Clear 불필요

    // 큐브맵(반사용): Forward 에서는 카메라가 스카이를 그리며 slot 3 에 걸어 줬지만, 라이팅은 스카이보다 먼저라 직접 찾는다
    static const _uint skyTexIndex = m_GameInstance->Find_Texture("SkyBox");

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("DeferredLightingPSO"));
    m_CommandList->SetGraphicsRootSignature(m_GameInstance->GetRootSignature("DeferredLightingRS"));
    m_CommandList->SetGraphicsRootDescriptorTable(0, m_GameInstance->Get_EngineSrvGpuHandle(SRV_SLOT_GBUFFER0));   // t0~t4 space2
    m_GameInstance->Set_CurrentFramePBMats(0);                                                                      // slot 1 PassCB, slot 2 머티리얼
    m_CommandList->SetGraphicsRootDescriptorTable(3, m_GameInstance->Get_TextureGpuHandle(skyTexIndex));          // t0 큐브맵
    m_CommandList->SetGraphicsRootDescriptorTable(4, m_GameInstance->m_ShadowMap->mhGpuSrv);                       // t1 그림자맵
    m_CommandList->SetGraphicsRoot32BitConstant(5, DebugMode, 0);                                                   // b2 디버그 보기

    // 동적 점광원 (포 발사, 폭발): CLightMgr 가 살아 있는 조명을 이번 프레임 FrameResource 버퍼에 복사
    // 프레임 리소스마다 버퍼가 따로라 GPU 가 아직 읽고 있는 이전 프레임 버퍼를 덮어쓰지 않는다
    CFrameResource* pFrame = m_GameInstance->Get_Current_FrameResource();
    const UINT pointLightCount = m_GameInstance->m_LightMgr->Upload(pFrame->m_PointLightBuffer, MAX_POINT_LIGHTS);
    m_CommandList->SetGraphicsRoot32BitConstant(6, pointLightCount, 0);                                             // b3 점광원 개수
    m_CommandList->SetGraphicsRootShaderResourceView(7, pFrame->m_PointLightBuffer->Resource()->GetGPUVirtualAddress()); // t5 space2
    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_CommandList->DrawInstanced(3, 1, 0, 0);

    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDepth,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    // (3) Forward: 스카이 + 반투명 ----------------------------------------------
    // 기하 패스가 남긴 깊이를 그대로 쓴다 → 스카이(LESS_EQUAL, 깊이 1)는 아무것도 안 그려진 픽셀에만, 반투명은 불투명 물체 뒤에서 가려진다
    D3D12_CPU_DESCRIPTOR_HANDLE hdrRtv = pHDR->Get_RTV();
    m_CommandList->OMSetRenderTargets(1, &hdrRtv, true, &dsv);
    Bind_DefaultRS_for_MainPass();          // 라이팅 RS 로 바꿨으니 DefaultRS 바인딩을 다시

    m_CommandList->SetPipelineState(m_GameInstance->GetPSO("SkyPSO"));
    Render_Priority();

    if (DebugMode == 0)     // G-Buffer 시각화 중에는 반투명을 빼서 G-Buffer 만 보이게
    {
        m_CommandList->SetPipelineState(m_GameInstance->GetPSO("EffectPSO"));
        Render_Blend();

        m_CommandList->SetPipelineState(m_GameInstance->GetPSO("BulletPathPSO"));
        Render_BulletPath();
    }
    else
    {
        // 그리지 않아도 목록은 비워야 한다 (참조 해제)
        for (auto& pGameObject : m_vRenderObjects[RG_BLEND])
            Safe_Release(pGameObject);
        m_vRenderObjects[RG_BLEND].clear();
        for (auto& pGameObject : m_vRenderObjects[RG_BULLETPATH])
            Safe_Release(pGameObject);
        m_vRenderObjects[RG_BULLETPATH].clear();
    }
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
