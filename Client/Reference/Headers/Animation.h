#pragma once

#include "Base.h"

/* 특정 애니메이션을 구동하기위한 정보를 가진다. */
/* 전체시간(Duration), 속도(TickPerSecond), 제어하는 뼈의 정보들. */


BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& rhs);
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	void Invalidate_TransformationMatrix(_bool isLoop, const vector<class CBone*>& Bones, _float fTimeDelta);

private:
	_char						m_szName[MAX_PATH] = "";
	_float						m_Duration = { 0.f };
	_float						m_fTickPerSecond = { 0.f };
	_float						m_CurrentAnimPosition = { 0.f };

	_uint						m_iNumChannels = { 0 };
	vector<class CChannel*>		m_Channels;

	_bool						m_isFinished = { false };
	vector<_uint>				m_CurrentKeyFrameIndices;

	

public:
	static CAnimation* Create(const aiAnimation* pAIAnimation, const vector<class CBone*>& Bones);
	CAnimation* Clone();
	virtual void Free() override;
};

END