#pragma once
#include "Base.h"

BEGIN(Engine)

/*---------------------------------------------------------------------------
    오프스크린 렌더타겟 하나 (리소스 + RTV + SRV + 현재 상태)

    - HDR 씬 RT, Bloom 핑퐁 RT 처럼 "그렸다가 다음 패스에서 텍스처로 읽는" 용도
    - 디스크립터 칸은 직접 만들지 않고 받아 쓴다
      RTV: CGameInstance::Get_EngineRtv(slot)
      SRV: CGameInstance::Get_EngineSrvCpuHandle / GpuHandle(slot)   (Engine_Config.h)
    - 리소스 상태를 CPU 쪽에서 기억해 Transition() 이 Before 상태를 알아서 넣는다
      (11/19 Barrier 실험의 #527 BEFORE_AFTER_MISMATCH 같은 실수 방지)
      명령 리스트 하나에 순서대로 기록한다는 전제다
---------------------------------------------------------------------------*/
class ENGINE_DLL CRenderTarget : public CBase
{
private:
	CRenderTarget() = default;
	virtual ~CRenderTarget() = default;

public:
	HRESULT Initialize(UINT width, UINT height, DXGI_FORMAT format, const FLOAT clearColor[4],
		D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE srvCpu, D3D12_GPU_DESCRIPTOR_HANDLE srvGpu);

	// 해상도가 바뀌면 리소스와 뷰를 다시 만든다 (디스크립터 칸은 그대로)
	// 호출 전에 GPU가 이 리소스를 다 썼는지(Flush) 보장해야 한다
	HRESULT Resize(UINT width, UINT height);

	// 현재 상태 → after 상태로 Barrier. 이미 같은 상태면 아무것도 안 한다
	void Transition(ID3D12GraphicsCommandList* pCommandList, D3D12_RESOURCE_STATES after);

	// 생성할 때 넣은 클리어 값과 같은 색으로 지운다 (다르면 Debug Layer 경고 + 빠른 클리어 못 씀)
	void Clear(ID3D12GraphicsCommandList* pCommandList);

	// 이 RT 하나만 출력으로 바인딩 (깊이 없음) + 뷰포트/시저를 이 RT 크기로. 절반 해상도 Bloom 패스용
	void Bind_as_Output(ID3D12GraphicsCommandList* pCommandList);

public:
	ID3D12Resource*					Get_Resource() const { return m_Resource; }
	D3D12_CPU_DESCRIPTOR_HANDLE		Get_RTV() const { return m_Rtv; }
	D3D12_GPU_DESCRIPTOR_HANDLE		Get_SRV() const { return m_SrvGpu; }
	D3D12_RESOURCE_STATES			Get_State() const { return m_State; }
	UINT							Get_Width() const { return m_Width; }
	UINT							Get_Height() const { return m_Height; }
	DXGI_FORMAT						Get_Format() const { return m_Format; }

private:
	HRESULT Create_Resource_and_Views();

private:
	ID3D12Resource*					m_Resource = nullptr;
	D3D12_RESOURCE_STATES			m_State = D3D12_RESOURCE_STATE_COMMON;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_Rtv = {};
	D3D12_CPU_DESCRIPTOR_HANDLE		m_SrvCpu = {};
	D3D12_GPU_DESCRIPTOR_HANDLE		m_SrvGpu = {};

	UINT							m_Width = 0;
	UINT							m_Height = 0;
	DXGI_FORMAT						m_Format = DXGI_FORMAT_UNKNOWN;
	FLOAT							m_ClearColor[4] = { 0.f, 0.f, 0.f, 1.f };

public:
	static CRenderTarget* Create(UINT width, UINT height, DXGI_FORMAT format, const FLOAT clearColor[4],
		D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE srvCpu, D3D12_GPU_DESCRIPTOR_HANDLE srvGpu);
	virtual void Free() override;
};

END
