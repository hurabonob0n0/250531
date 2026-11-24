#pragma once
#include "Base.h"
#include "UploadBuffer.hpp"

BEGIN(Engine)

/*---------------------------------------------------------------------------
    수명이 있는 동적 점광원 목록 (Deferred 2주차 후반)

    - 포 발사(머즐 플래시), 포탄 폭발처럼 잠깐 켜졌다 꺼지는 조명용
    - Add_PointLight 로 넣으면 Duration 동안 밝기가 (1 - t)^2 로 줄다가 사라진다
    - 매 프레임 Upload 가 살아 있는 조명을 현재 FrameResource 의 StructuredBuffer 에 복사한다
      → Deferred 라이팅 패스가 gPointLights[0 ~ count-1] 을 픽셀마다 돈다
    - GPU 쪽 형식은 기존 Light 구조체 그대로 (LightingUtil.hlsl 의 ComputePointLight 재사용)
    - Color 는 1 을 넘어도 된다 (HDR). 1 을 넘는 부분이 Bloom bright pass 에 걸린다
---------------------------------------------------------------------------*/
class ENGINE_DLL CLightMgr : public CBase
{
public:
	struct POINTLIGHT_DESC
	{
		XMFLOAT3	Position = { 0.f, 0.f, 0.f };
		XMFLOAT3	Color = { 1.f, 1.f, 1.f };	// 시작 밝기 (HDR, 1 넘어도 됨)
		float		Radius = 10.f;				// 이 거리에서 0 (선형 감쇠)
		float		Duration = 0.5f;			// 초. 0 이하면 꺼지지 않는다
		int			Group = 0;					// 묶음 번호. Remove_Group 으로 한 번에 지운다 (P 키 테스트 조명 = 1)
	};

private:
	CLightMgr() = default;
	virtual ~CLightMgr() = default;

public:
	void Add_PointLight(const POINTLIGHT_DESC& desc);
	void Update(float fTimeDelta);								// 나이 증가, 수명 끝난 조명 제거
	UINT Upload(CUploadBuffer<Light>* pBuffer, UINT maxCount);	// 살아 있는 조명을 버퍼에 복사, 복사한 개수 반환
	UINT Get_Count() const { return static_cast<UINT>(m_PointLights.size()); }
	void Clear() { m_PointLights.clear(); }
	void Remove_Group(int group);

private:
	struct ACTIVE_LIGHT
	{
		POINTLIGHT_DESC	Desc;
		float			Age = 0.f;
	};
	vector<ACTIVE_LIGHT>	m_PointLights;

public:
	static CLightMgr* Create();
	virtual void Free() override;
};

END
