#include "Graphic_Dev.h"

IMPLEMENT_SINGLETON(CGraphic_Dev)

CGraphic_Dev::CGraphic_Dev() {};

void CGraphic_Dev::Initialize(WindowInfo windowInfo)
{
    if(FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory))))
        MSG_BOX("Failed to Create : dxgiFactory");

    HRESULT hardwareResult = D3D12CreateDevice(                 
        nullptr,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_Device)
    );

    Set_WindowInfo(windowInfo);

    //if (FAILED(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))))
    //    MSG_BOX("Failed to Create : Fence");

    m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);                         
    m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = m_BackBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    if(FAILED(m_Device->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels,
        sizeof(msQualityLevels))))
        MSG_BOX("Failed to Check : Features");

    m4xMsaaQuality = msQualityLevels.NumQualityLevels;
    assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
    LogAdapters();
#endif

    CreateCommandObjects();
    CreateSwapChain();
    CreateRtvAndDsvDescriptorHeaps();
}

void CGraphic_Dev::CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    if (FAILED(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue))))
        MSG_BOX("Failed to Create : CommandQueue");

    if (FAILED(m_Device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&m_CommandListAlloc))))
        MSG_BOX("Failed to Create : CommandListAllocator");

    if (FAILED(m_Device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_CommandListAlloc,
        nullptr,
        IID_PPV_ARGS(&m_CommandList))))
        MSG_BOX("Failed to Create : CommandList");

    m_CommandList->Close();
}

void CGraphic_Dev::CreateSwapChain()
{
    Safe_Release(m_SwapChain);

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = m_ClientWidth;
    sd.BufferDesc.Height = m_ClientHeight;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = m_BackBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SwapChainBufferCount;
    sd.OutputWindow = m_hMainWnd;
    sd.Windowed = !m_isFullScreen;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // 참고: 스왑 체인은 큐를 이용해서 플러시를 수행합니다.
    if(FAILED(m_dxgiFactory->CreateSwapChain(
        m_CommandQueue,
        &sd,
        &m_SwapChain)))
        MSG_BOX("Failed to Create : SwapChain");
}

void CGraphic_Dev::CreateRtvAndDsvDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    if(FAILED(m_Device->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap))))
        MSG_BOX("Failed to Create : RTVDescriptorHeap");

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    if (FAILED(m_Device->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(&m_DsvHeap))))
        MSG_BOX("Failed to Create : DSVDescriptorHeap");
}

void CGraphic_Dev::Execute_CommandList()
{
    if (FAILED(m_CommandList->Close()))
        MSG_BOX("Failed to Close : CommandList");
    ID3D12CommandList* cmdLists[] = { m_CommandList };
    m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
}

void CGraphic_Dev::Set_WindowInfo(WindowInfo windowInfo)
{
    m_ClientWidth = windowInfo.Wincx;
    m_ClientHeight = windowInfo.Wincy;
    m_isFullScreen = windowInfo.isFullScreen;
    m_hMainWnd = windowInfo.windowHandle;
}

void CGraphic_Dev::OnResize()
{
    assert(m_Device);
    assert(m_SwapChain);
    assert(m_CommandListAlloc);

    if (FAILED(m_CommandList->Reset(m_CommandListAlloc, nullptr)))
        MSG_BOX("Failed to Reset CommandList");

    // 다시 생성하기 전에 이전에 생성된 리소스들을 해제합니다.
    for (int i = 0; i < SwapChainBufferCount; ++i)
        Safe_Release(m_SwapChainBuffer[i]);

    Safe_Release(m_DepthStencilBuffer);

    // 스왑체인의 크기를 변경합니다.
    if (FAILED((m_SwapChain->ResizeBuffers(
        SwapChainBufferCount,
        m_ClientWidth, m_ClientHeight,
        m_BackBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH))))
        MSG_BOX("Failed to ResizeBuffer");

    m_CurrBackBuffer = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < SwapChainBufferCount; ++i)
    {
        if (FAILED(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i]))))
            MSG_BOX("Failed to GetBuffer : BackBuffer");
        m_Device->CreateRenderTargetView(m_SwapChainBuffer[i], nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
    }

    // 뎁스/스텐실 버퍼와 뷰를 생성합니다.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_ClientWidth;
    depthStencilDesc.Height = m_ClientHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;

    // 수정 2016년 11월 12일: SSAO는 뎁스 버퍼를 읽기 위해 SRV가 필요하다.
    // 그러므로 동일한 리소스에 두 개의 뷰를 생성해야 한다.
    //  1. SRV 포멧: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
    //  2. DSV 포멧: DXGI_FORMAT_D24_UNORM_S8_UINT
    // 그러므로 뎁스/스텐실을 생성할 때 타입 없는 포멧을 사용한다.
    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

    depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = m_DepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    if (FAILED(m_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(&m_DepthStencilBuffer))))
        MSG_BOX("Failed to Create : DSView");

    // 리소스의 포멧을 사용하여 밉맵 0에 해당하는 디스크립터를 생성한다.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = m_DepthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    m_Device->CreateDepthStencilView(m_DepthStencilBuffer, &dsvDesc, DepthStencilView());

    // 리소스를 초기 상태에서 뎁스 버퍼로 사용할 수 있는 상태로 변경한다.
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer,
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_DEPTH_WRITE));

    // 크기 변경을 위한 명령들을 실행시킨다.
    if (FAILED(m_CommandList->Close()))
        MSG_BOX("Failed to Close : CommandList");
    ID3D12CommandList* cmdLists[] = { m_CommandList };
    m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

    // 뷰포트를 업데이트한다.
    m_ScreenViewport.TopLeftX = 0;
    m_ScreenViewport.TopLeftY = 0;
    m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
    m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
    m_ScreenViewport.MinDepth = 0.0f;
    m_ScreenViewport.MaxDepth = 1.0f;

    m_ScissorRect = { 0, 0, m_ClientWidth, m_ClientHeight };
}

//void CGraphic_Dev::FlushCommandQueue()
//{
//    m_CurrentFence++;
//
//    // 커맨드들의 처리는 GPU에서 진행되기 때문에 언제 커맨드들이 처리됬는지를 CPU에서 알기 힘듭니다.
//    // 그러므로 모든 커맨드가 처리됬을 때 새 펜스 지점을 설정하는 인스트럭션을 커맨드 큐에 추가합니다.
//    // Signal()을 호출하기 전에 제출한 커맨드들이 처리되기 전까지 새 펜스 지점은 설정되지 않습니다.
//    if (FAILED(m_CommandQueue->Signal(m_Fence, m_CurrentFence)))
//        MSG_BOX("Failed to Signal : Fence");
//
//    // GPU가 새 펜스 지점까지의 명령들을 완료할 때까지 기다립니다.
//    if (m_Fence->GetCompletedValue() < m_CurrentFence)
//    {
//        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
//
//        // GPU가 새 펜스 지점에 도달했으면 이벤트를 발동시킵니다.
//        if (FAILED(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle)))
//            MSG_BOX("Failed to Create : Event");
//
//        // GPU가 새 펜스를 설정하고 이벤트가 발동될때까지 기다립니다.
//        WaitForSingleObject(eventHandle, INFINITE);
//        CloseHandle(eventHandle);
//    }
//}

ID3D12Resource* CGraphic_Dev::CurrentBackBuffer() const
{
    return m_SwapChainBuffer[m_CurrBackBuffer];
}

D3D12_CPU_DESCRIPTOR_HANDLE CGraphic_Dev::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_CurrBackBuffer,
        m_RtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE CGraphic_Dev::DepthStencilView() const
{
    return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void CGraphic_Dev::LogAdapters()
{
    UINT i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wstring text = L"***Adapter: ";
        text += desc.Description;
        text += L"\n";

        OutputDebugString(text.c_str());

        adapterList.push_back(adapter);

        ++i;
    }

    for (size_t i = 0; i < adapterList.size(); ++i)
    {
        LogAdapterOutputs(adapterList[i]);
        Safe_Release(adapterList[i]);
    }
}

void CGraphic_Dev::LogAdapterOutputs(IDXGIAdapter* adapter)
{
    UINT i = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::wstring text = L"***Output: ";
        text += desc.DeviceName;
        text += L"\n";

        OutputDebugString(text.c_str());

        LogOutputDisplayModes(output, m_BackBufferFormat);
        Safe_Release(output);

        ++i;
    }
}

void CGraphic_Dev::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    UINT count = 0;
    UINT flags = 0;

    // 디스플레이 모드의 개수를 얻기위해 nullptr과 함께 호출합니다.
    output->GetDisplayModeList(format, flags, &count, nullptr);

    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (auto& x : modeList)
    {
        UINT n = x.RefreshRate.Numerator;
        UINT d = x.RefreshRate.Denominator;
        std::wstring text =
            L"Width = " + std::to_wstring(x.Width) + L" " +
            L"Height = " + std::to_wstring(x.Height) + L" " +
            L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
            L"\n";

        OutputDebugString(text.c_str());
    }
}

void CGraphic_Dev::Set_BackBuffer_and_DSV()
{
    // 리소스의 상태를 렌더링을 할 수 있도록 변경합니다.
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET));

    // 뷰포트와 시져를 설정한다. 커맨드 리스트가 리셋되고 나서 설정해야만 합니다.
    m_CommandList->RSSetViewports(1, &m_ScreenViewport);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

    // 백 버퍼와 뎁스 버퍼를 클리어 합니다.
    m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // 어디에 렌더링을 할지 설정합니다.
    m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
}

void CGraphic_Dev::Present()
{
    // 리소스의 상태를 출력할 수 있도록 변경합니다.
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT));

    // 커맨드 기록을 종료합니다.
    m_CommandList->Close();

    // 커맨드 리스트의 실행을 위해 큐에 제출합니다.
    ID3D12CommandList* cmdLists[] = { m_CommandList };
    m_CommandQueue->ExecuteCommandLists(1, cmdLists);

    // 백 버퍼와 프론트 버퍼를 교체합니다.
    m_SwapChain->Present(0, 0);
    m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

}

void CGraphic_Dev::Free()
{
    for (int i = 0; i < SwapChainBufferCount; ++i)
        Safe_Release(m_SwapChainBuffer[i]);

    Safe_Release(m_DepthStencilBuffer);

    Safe_Release(m_RtvHeap);
    Safe_Release(m_DsvHeap);

    Safe_Release(m_SwapChain);
    Safe_Release(m_CommandList);
    Safe_Release(m_CommandListAlloc);
    Safe_Release(m_CommandQueue);

    Safe_Release(m_Device);
    Safe_Release(m_dxgiFactory);
#if defined(DEBUG) | defined(_DEBUG)
    // D3D12 디버그 레이어를 활성화 합니다.
    {
        ID3D12Debug* debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();  // 디버그 레이어 활성화
            debugController->Release();
        }
        if (debugController != nullptr) debugController->Release();
    } // 메모리 누수 디버깅 체크용 코드
#endif
}
