#include "..\Public\Bone.h"

CBone::CBone()
{
}

HRESULT CBone::Initialize(const aiNode * pAINode, _int iParentBoneIndex)
{
	strcpy_s(m_szName, pAINode->mName.data);

	aiMatrix4x4 mat = pAINode->mTransformation;

	if(strstr(m_szName, "RootNode"))
		aiMatrix4x4::RotationY(XM_PIDIV2, mat); // RootNode는 Y축 기준으로 90도 회전

	if (strstr(m_szName, "$AssimpFbx$_Translation")) {
		mat.a4 /= 100.f; // X
		mat.b4 /= 100.f; // Y
		mat.c4 /= 100.f; // Z
	}

	memcpy(&m_TransformationMatrix, &mat, sizeof(_float4x4));

	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));

	XMStoreFloat4x4(&m_CombindTransformationMatrix, XMMatrixIdentity());

	m_iParentBoneIndex = iParentBoneIndex;

	//std::ofstream fout("../bin/BoneInfo.txt", std::ios::app); // append 모드
	//if (fout.is_open())
	//{
	//	fout << g_BoneNum++ << "." << " BoneName: " << m_szName << ", ParentIndex: " << m_iParentBoneIndex << std::endl;
	//	float* fValue = (float*) & m_TransformationMatrix;
	//	for (int i = 0; i < 16; ++i)
	//	{
	//		fout << *fValue++ << ", ";
	//		if (i % 4 == 3)
	//			fout << '\n';
	//	}
	//	fout.close();
	//}
	
	return S_OK;


	//strcpy_s(m_szName, pAINode->mName.data);

	//aiMatrix4x4 mat;

	//if (strstr(m_szName, "$AssimpFbx$_PreRotation")) {
	//	// Z축 기준 -90도 회전 행렬을 생성
	//	float angle = -XM_PIDIV2; // -90도
	//	aiMatrix4x4::RotationZ(angle, mat);
	//}
	//else {
	//	mat = pAINode->mTransformation;
	//}

	//memcpy(&m_TransformationMatrix, &mat, sizeof(_float4x4));
	//XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));

	//XMStoreFloat4x4(&m_CombindTransformationMatrix, XMMatrixIdentity());

	//m_iParentBoneIndex = iParentBoneIndex;

	//return S_OK;
}

void CBone::Invalidate_CombinedTransformationMatrix(const vector<CBone*>& Bones)
{
	/*if (-1 == m_iParentBoneIndex)
		m_CombindTransformationMatrix = m_TransformationMatrix;

	else*/
	if(m_iParentBoneIndex != -1)
	{
		/* m_CombindTransformationMatrix  = 나만의 상태행렬(TransformationMatrix) * 부모의 최종행렬(Parent`s CombinedMatrix) */
		XMStoreFloat4x4(&m_CombindTransformationMatrix, 
			XMLoadFloat4x4(&m_TransformationMatrix) * 
			XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombindTransformationMatrix));
	}
}

CBone * CBone::Create(const aiNode * pAINode, _int iParentBoneIndex)
{
	CBone* pInstance = new CBone();

	if (FAILED(pInstance->Initialize(pAINode, iParentBoneIndex)))
	{
		MSG_BOX("Failed to Created : CBone");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CBone * CBone::Clone()
{
	return new CBone(*this);
}

void CBone::Free()
{
	
}
