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
	_float4x4 Get_CombinedMatrix();

	//_float4x4 Get_TransformMatrix() { return m_Bone->Get_TransformMatrix(); }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

public:
	void Set_Bone(const vector<class CBone*> Bones);

public:
	void Set_Team(int team);

public:
	void Update();
	virtual HRESULT Render();

public:
	HRESULT Bind_CBView_On_Shader();
	void Set_MaterialIndexOffset(_uint MatOffset);

public:
	void Set_MaterialIndex(_uint Matindex);

public:
	/* Track bending. Set to a zero SagB.w to turn it off (that is the default). */
	void Set_TrackSag(const _float4& SagA, const _float4& SagB, const _float4& Param) {
		m_TrackSagA = SagA; m_TrackSagB = SagB; m_TrackParam = Param;
	}

public:
	char				m_szName[MAX_PATH] = "";
	_uint				m_iMaterialIndex = { 0 };
	CBone*				m_Bone = nullptr;
	class CBBinding*	m_CBBinding = nullptr;

private:
	_float4				m_TrackSagA = {};
	_float4				m_TrackSagB = {};
	_float4				m_TrackParam = {};

private:
#if USE_ASSIMP_BAKE
	HRESULT Ready_NonAnim_Mesh(const aiMesh* pAIMesh, _fmatrix PivotMatrix);
	HRESULT Ready_Anim_Mesh(const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PivotMatrix);
#endif

public:
	static CMesh* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;

public:
	static int g_MeshIndex;
};

END