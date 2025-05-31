#pragma once

#include "Base.h"

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

public:
	HRESULT Initialize(const aiNode* pAINode, _int iParentBoneIndex);
	void Invalidate_CombinedTransformationMatrix(const vector<CBone*>& Bones);

private:
	_char				m_szName[MAX_PATH] = "";
	_float4x4			m_TransformationMatrix; /* 현재 뼈만의 상태 */
	_float4x4			m_CombindTransformationMatrix; /* 현재 뼈만의 상태 * 부모뼈의 상태(m_CombindTransformationMatrix) */
	
	/* 부모뼈에 대한 상태를 저장해놓는다. */		
	_int				m_iParentBoneIndex = { 0 };

public:
	static CBone* Create(const aiNode* pAINode, _int iParentBoneIndex);
	CBone* Clone();
	virtual void Free() override;
};

END