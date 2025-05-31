#pragma once

/* 모델을 구성하는 하나의 파츠. */
/* 정점과 인덱스를 구성하기위해 디자인된 클래스다. */
#include "VIBuffer.h"
#include "Model.h"


BEGIN(Engine)

class CMesh final : public CVIBuffer
{
private:
	CMesh(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext);
	CMesh(CMesh& rhs);
	virtual ~CMesh() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	virtual HRESULT Initialize_Prototype(CModel::TYPE eModelType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PivotMatrix);
	virtual HRESULT Initialize(void* pArg);

public:
	void Set_Bone(const vector<class CBone*> Bones);

public:
	void Update();
	virtual HRESULT Render();

public:
	HRESULT Bind_CBView_On_Shader();
	void Set_MaterialIndexOffset(_uint MatOffset);

public:
	void Set_MaterialIndex(_uint Matindex);

private:
	char				m_szName[MAX_PATH] = "";
	_uint				m_iMaterialIndex = { 0 };
	CBone*				m_Bone = nullptr;
	class CBBinding*	m_CBBinding = nullptr;

private:
	HRESULT Ready_NonAnim_Mesh(const aiMesh* pAIMesh, _fmatrix PivotMatrix);
	HRESULT Ready_Anim_Mesh(const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PivotMatrix);

public:
	static CMesh* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext, CModel::TYPE eModelType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PivotMatrix);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END