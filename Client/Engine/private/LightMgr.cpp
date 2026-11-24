#include "LightMgr.h"

void CLightMgr::Add_PointLight(const POINTLIGHT_DESC& desc)
{
	ACTIVE_LIGHT light;
	light.Desc = desc;
	light.Age = 0.f;
	m_PointLights.push_back(light);
}

void CLightMgr::Update(float fTimeDelta)
{
	for (auto& light : m_PointLights)
		light.Age += fTimeDelta;

	// 수명이 끝난 조명 제거 (Duration <= 0 은 영구 조명)
	m_PointLights.erase(
		std::remove_if(m_PointLights.begin(), m_PointLights.end(),
			[](const ACTIVE_LIGHT& light) { return light.Desc.Duration > 0.f && light.Age >= light.Desc.Duration; }),
		m_PointLights.end());
}

void CLightMgr::Remove_Group(int group)
{
	m_PointLights.erase(
		std::remove_if(m_PointLights.begin(), m_PointLights.end(),
			[group](const ACTIVE_LIGHT& light) { return light.Desc.Group == group; }),
		m_PointLights.end());
}

UINT CLightMgr::Upload(CUploadBuffer<Light>* pBuffer, UINT maxCount)
{
	UINT count = 0;
	for (const auto& light : m_PointLights)
	{
		if (count >= maxCount)
			break;	// 넘치는 조명은 버린다 (MAX_POINT_LIGHTS)

		// 밝기: 처음에 가장 밝고 (1 - t)^2 로 빠르게 줄었다가 끝에서 0. 폭발 빛이 "번쩍" 하고 사그라드는 느낌
		float fade = 1.f;
		if (light.Desc.Duration > 0.f)
		{
			const float t = light.Age / light.Desc.Duration;
			fade = (1.f - t) * (1.f - t);
		}

		Light gpuLight;
		gpuLight.Strength = XMFLOAT3(light.Desc.Color.x * fade, light.Desc.Color.y * fade, light.Desc.Color.z * fade);
		gpuLight.FalloffStart = 0.f;				// 중심부터 선형으로 줄어든다 (CalcAttenuation)
		gpuLight.FalloffEnd = light.Desc.Radius;	// 이 거리 밖은 ComputePointLight 가 바로 0 을 반환
		gpuLight.Position = light.Desc.Position;
		gpuLight.Direction = XMFLOAT3(0.f, -1.f, 0.f);
		gpuLight.SpotPower = 0.f;

		pBuffer->CopyData(static_cast<int>(count), gpuLight);
		++count;
	}
	return count;
}

CLightMgr* CLightMgr::Create()
{
	return new CLightMgr;
}

void CLightMgr::Free()
{
	m_PointLights.clear();
}
