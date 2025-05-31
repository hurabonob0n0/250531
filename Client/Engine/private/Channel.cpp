#include "..\Public\Channel.h"
#include "Bone.h"

CChannel::CChannel()
{

}

HRESULT CChannel::Initialize(const aiNodeAnim * pAIChannel, const vector<class CBone*>& Bones)
{
	strcpy_s(m_szName, pAIChannel->mNodeName.data);

	_uint		iBoneIndex = 0;

	auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool {
		if (0 == strcmp(pBone->Get_BoneName(), m_szName))
			return true;
		++iBoneIndex;
		return false;
	});

	m_iBoneIndex = iBoneIndex;


	m_iNumKeyFrames = max(pAIChannel->mNumScalingKeys, pAIChannel->mNumRotationKeys);
	m_iNumKeyFrames = max(m_iNumKeyFrames, pAIChannel->mNumPositionKeys);

	KEYFRAME			KeyFrame = {};

	_float4				vScale = {}, vRotation = {}, vTranslation = {0.0f, 0.f, 0.f, 1.f};

	for (size_t i = 0; i < m_iNumKeyFrames; i++)
	{
		if (i < pAIChannel->mNumScalingKeys) 
		{
			memcpy(&vScale, &pAIChannel->mScalingKeys[i].mValue, sizeof(_float3));
			KeyFrame.fTime = pAIChannel->mScalingKeys[i].mTime;
		}

		if (i < pAIChannel->mNumRotationKeys)
		{
			/*memcpy(&vRotation, &pAIChannel->mRotationKeys[i].mValue, sizeof(_float4));*/
			vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
			vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
			vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
			vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;

			KeyFrame.fTime = pAIChannel->mRotationKeys[i].mTime;
		}

		if (i < pAIChannel->mNumPositionKeys)
		{
			memcpy(&vTranslation, &pAIChannel->mPositionKeys[i].mValue, sizeof(_float3));
			KeyFrame.fTime = pAIChannel->mPositionKeys[i].mTime;
		}
		
		KeyFrame.vScale = vScale;
		KeyFrame.vRotation = vRotation;
		KeyFrame.vTranslation = vTranslation;

		m_KeyFrames.push_back(KeyFrame);
	}

	return S_OK;
}

void CChannel::Invalidate_TransformationMatrix(const vector<class CBone*>& Bones, _uint* pCurrentKeyFrameIndex, _float fCurrentAnimPosition)
{
	if (0.0f == fCurrentAnimPosition)
		*pCurrentKeyFrameIndex = 0;
	KEYFRAME		LastKeyFrame = m_KeyFrames.back();

	_vector			vScale;
	_vector			vRotation;
	_vector			vTranslation;

	if (fCurrentAnimPosition >= LastKeyFrame.fTime)
	{
		vScale = XMLoadFloat4(&LastKeyFrame.vScale);
		vRotation = XMLoadFloat4(&LastKeyFrame.vRotation);
		vTranslation = XMLoadFloat4(&LastKeyFrame.vTranslation);
	}
	else
	{
		if (fCurrentAnimPosition >= m_KeyFrames[*pCurrentKeyFrameIndex + 1].fTime)
			++*pCurrentKeyFrameIndex;

		_float		fRatio = (fCurrentAnimPosition - m_KeyFrames[*pCurrentKeyFrameIndex].fTime) /
			(m_KeyFrames[*pCurrentKeyFrameIndex + 1].fTime - m_KeyFrames[*pCurrentKeyFrameIndex].fTime);


		_vector		vSourScale, vDestScale;
		_vector		vSourRotation, vDestRotation;
		_vector		vSourTranslation, vDestTranslation;

		vSourScale = XMLoadFloat4(&m_KeyFrames[*pCurrentKeyFrameIndex].vScale);
		vDestScale = XMLoadFloat4(&m_KeyFrames[*pCurrentKeyFrameIndex + 1].vScale);		
		vScale = XMVectorLerp(vSourScale, vDestScale, fRatio);

		vSourRotation = XMLoadFloat4(&m_KeyFrames[*pCurrentKeyFrameIndex].vRotation);
		vDestRotation = XMLoadFloat4(&m_KeyFrames[*pCurrentKeyFrameIndex + 1].vRotation);
		vRotation = XMQuaternionSlerp(vSourRotation, vDestRotation, fRatio);

		vSourTranslation = XMLoadFloat4(&m_KeyFrames[*pCurrentKeyFrameIndex].vTranslation);
		vDestTranslation = XMLoadFloat4(&m_KeyFrames[*pCurrentKeyFrameIndex + 1].vTranslation);
		vTranslation = XMVectorLerp(vSourTranslation, vDestTranslation, fRatio);	
	}
	_matrix			TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vTranslation);

	/* 이 뼈의 특정시간에 맞는 상태를 모델이 가지고 있는 뼈에 던져준다. */
	Bones[m_iBoneIndex]->Set_TransformationMatrix(TransformationMatrix);

}

CChannel * CChannel::Create(const aiNodeAnim * pAIChannel, const vector<class CBone*>& Bones)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(pAIChannel, Bones)))
	{
		MSG_BOX("Failed to Created : CChannel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CChannel::Free()
{
	m_KeyFrames.clear();
}
