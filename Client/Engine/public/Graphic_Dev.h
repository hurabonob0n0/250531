#pragma once
#include "Base.h"

BEGIN(Engine)

class CGraphic_Dev : public CBase
{
	DECLARE_SINGLETON(CGraphic_Dev)

private:
	CGraphic_Dev();
	virtual ~CGraphic_Dev() = default;

public:
	ID3D12GraphicsCommandList*	Get_CommandList() { return m_CommandList; }
	ID3D12CommandQueue*			Get_CommandQueue() { return m_CommandQueue; }
	ID3D12Device*				Get_Device() { return m_Device; }
	UINT						Get_CBVUAVSRVHeapSize() { return m_CbvSrvUavDescriptorSize; }
	void Execute_CommandList();

#pragma region Initialize GraphicDev
public:
	void Initialize(WindowInfo windowInfo);

private:
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();


public:
	void Set_WindowInfo(WindowInfo windowInfo);
	void OnResize();

#pragma endregion

public: // For SwapChain
	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

public: //For Debug
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

public: //For Draw
	void Set_BackBuffer_and_DSV();
	void Present();

private:
	bool						m4xMsaaState = false;
	UINT						m4xMsaaQuality = 0;

	IDXGIFactory4*				m_dxgiFactory = nullptr;
	IDXGISwapChain*				m_SwapChain = nullptr;
	ID3D12Device*				m_Device = nullptr;

	ID3D12CommandQueue*			m_CommandQueue = nullptr;
	ID3D12CommandAllocator*		m_CommandListAlloc = nullptr;
	ID3D12GraphicsCommandList*	m_CommandList = nullptr;

	static const int            SwapChainBufferCount = 2;
	int                         m_CurrBackBuffer = 0;
	ID3D12Resource*				m_SwapChainBuffer[SwapChainBufferCount]{ nullptr };
	ID3D12Resource*				m_DepthStencilBuffer = nullptr;

	ID3D12DescriptorHeap*		m_RtvHeap = nullptr;
	ID3D12DescriptorHeap*		m_DsvHeap = nullptr;

	D3D12_VIEWPORT              m_ScreenViewport;
	D3D12_RECT                  m_ScissorRect;

	UINT                        m_RtvDescriptorSize = 0;
	UINT                        m_DsvDescriptorSize = 0;
	UINT                        m_CbvSrvUavDescriptorSize = 0;

	D3D_DRIVER_TYPE             m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT                 m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT                 m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//Window Info
	int                         m_ClientWidth = 1920;
	int                         m_ClientHeight = 1080;
	HWND                        m_hMainWnd = nullptr;
	bool                        m_isFullScreen = false;

public:
	void Free()override;
};

END