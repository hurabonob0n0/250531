#pragma once

#include "Base.h"

/* 특정 애니메이션에서 구동하는 뼈의 정보. */
/* 애니메이션이 전체 재생되는 동안에 각각의 시간대에 따른 이 뼈의 상태들를 저장한다. */

BEGIN(Engine)

class CChannel final : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	void Invalidate_TransformationMatrix(const vector<class CBone*>& Bones, _uint* pCurrentKeyFrameIndex, _float fCurrentAnimPosition);

private:
	_char					m_szName[MAX_PATH] = "";

	/* 현재 내 뼈는 이 애니메이션을 재생할때 어떤 키프레임까지 증가하여 상태를 표현하고 있었는가?! */
	// _uint					m_iCurrentKeyFrame = { 0 };
	_uint					m_iNumKeyFrames = { 0 };
	vector<KEYFRAME>		m_KeyFrames;
	_uint					m_iBoneIndex = { 0 };

public:
	static CChannel* Create(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	virtual void Free() override;
};

END