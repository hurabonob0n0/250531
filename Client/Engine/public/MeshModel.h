#pragma once

/* 어떤 모델이든 다수의 메시로 구성되어 있다. */
/* 모델 = 메시 + 메시 + 메시... */
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CMeshModel final : public CComponent
{
private:
	CMeshModel(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	CMeshModel(const CMeshModel& rhs);
	virtual ~CMeshModel() = default;

public:
	_uint Get_NumMeshes() const {
		return m_iNumMeshes;
	}

public:
	virtual HRESULT Initialize_Prototype(const string& strModelFilePath, _fmatrix PivotMatrix = XMMatrixIdentity(), _uint type = 0);
	virtual HRESULT Initialize(void* pArg);
	void Update();
	HRESULT Render(_uint iMeshIndex);


private:
	Assimp::Importer			m_Importer;
	const aiScene* m_pAIScene = { nullptr };

private:
	_uint						m_iNumMeshes = { 0 };
	vector< class CVIBuffer_Mesh*>			m_Meshes;

private:
	HRESULT Ready_Meshes(_fmatrix PivotMatrix = XMMatrixIdentity(), _uint type = 0);

public:
	static CMeshModel* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext, const string& strModelFilePath , _fmatrix PivotMatrix = XMMatrixIdentity(), _uint type = 0);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;


};

END

