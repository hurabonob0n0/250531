#pragma once

/* 어떤 모델이든 다수의 메시로 구성되어 있다. */
/* 모델 = 메시 + 메시 + 메시... */
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CModel final : public CComponent
{
public:
	enum TYPE { TYPE_NONANIM, TYPE_ANIM, TYPE_END};
private:
	CModel(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	CModel(const CModel& rhs);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const {
		return m_iNumMeshes;
	}

public:
	virtual HRESULT Initialize_Prototype(TYPE eModelType, const string& strModelFilePath, _fmatrix PivotMatrix);
	virtual HRESULT Initialize(void* pArg);

public:
	void Set_MatOffsets(_uint MatOffset);

public:
	void Set_MatIndex(_uint MeshIndex, _uint MatIndex);

public:
	void Invalidate_Bones();
	void Update();
	HRESULT Render(_uint iMeshIndex);
	void Make_Root_Combined_Matrix(_fmatrix WorldMat = XMMatrixIdentity());


private:
	/* .fbx파일로부터 읽어온 정보를 1차적으로 저장한다.*/
	Assimp::Importer			m_Importer;

	// m_pAIScene = m_Importer.ReadFile( 경로, 플래그 );
	const aiScene*				m_pAIScene = { nullptr };

private:
	TYPE						m_eModelType = { TYPE_END };
	_float4x4					m_PivotMatrix;

	_uint						m_iNumMeshes = { 0 };

	vector< class CMesh*>			m_Meshes;

	vector< class CBone*>			m_Bones;

	/* 1차적으로 저장된 데이터로부터 내가 사용할 정보를 꺼내와서 추후 사용하기 편하도록 내 스타일대로 정리한다. */
private:	
	HRESULT Ready_Meshes();
	HRESULT Ready_Bones(aiNode* pNode, _int iParentBoneIndex);

public:
	void Set_Matrix_to_Bone(_uint iBoneIndex,_matrix mat);

public:
	static CModel* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext, TYPE eModelType, const string& strModelFilePath, _fmatrix PivotMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

	
};

END

