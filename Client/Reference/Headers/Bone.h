#pragma once

#include "Base.h"
#include "Engine_Config.h"

/* Assimp : aiBone, aiNode, aiNodeAnim*/

/* aiBone : 이 뼈는 어떤 정점들에게 몇 퍼센트(0 ~ 1)나 영향을 주는가? */
/* aiNode : 이 뼈의 상속관계를 표현하고 이 뼈의 상태정보를 표현한다. */

BEGIN(Engine)

class CBone final : public CBase
{
private:
	CBone();
	virtual ~CBone() = default;

public:
	const _char* Get_BoneName() const {
		return m_szName;
	}

	const _float4x4* Get_CombinedMatrix() const {
		return &m_CombindTransformationMatrix;
	}

	_float4x4 Get_CombinedMatrix() {
		return m_CombindTransformationMatrix;
	}

	_matrix Get_TransformMatrix() {
		_matrix transformMatrix = XMLoadFloat4x4(&m_TransformationMatrix);
		return transformMatrix;
	}

	_matrix Get_BoneMatrix() const {
		return XMLoadFloat4x4(&m_TransformationMatrix);
	}

	void Set_CombinedMatrix(_fmatrix CombinedMatrix) {
		XMStoreFloat4x4(&m_CombindTransformationMatrix, CombinedMatrix);
	}

public:
	void Set_TransformationMatrix(_fmatrix TransformationMatrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);	
	}
	void Mul_TransformationMatrix(_fmatrix Transformmatrix)
	{
		XMStoreFloat4x4(&m_TransformationMatrix, (XMLoadFloat4x4(&m_TransformationMatrix) * Transformmatrix));
	}
	void Mul_CombinedTransformationMatrix(_fmatrix CombinedTransformmatrix)
	{
		XMStoreFloat4x4(&m_CombindTransformationMatrix, (CombinedTransformmatrix * XMLoadFloat4x4(&m_CombindTransformationMatrix)));
	}

public:
#if USE_ASSIMP_BAKE
	HRESULT Initialize(const aiNode* pAINode, _int iParentBoneIndex);
#endif
	HRESULT Initialize(const char* szName, const _float4x4& transformMat, _int iParentIndex);
	void Invalidate_CombinedTransformationMatrix(const vector<CBone*>& Bones);

public:
	_char				m_szName[MAX_PATH] = "";
	_float4x4			m_TransformationMatrix; /* 현재 뼈만의 상태 */
	_float4x4			m_CombindTransformationMatrix; /* 현재 뼈만의 상태 * 부모뼈의 상태(m_CombindTransformationMatrix) */
	
	/* 부모뼈에 대한 상태를 저장해놓는다. */		
	_int				m_iParentBoneIndex = { 0 };

public:
#if USE_ASSIMP_BAKE
	static CBone* Create(const aiNode* pAINode, _int iParentBoneIndex);
#endif
	static CBone* Create(const char* szName, const _float4x4& transformMat, _int iParentIndex);
	CBone* Clone();
	virtual void Free() override;
};

END