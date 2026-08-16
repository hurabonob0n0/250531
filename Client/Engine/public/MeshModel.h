#pragma once

/* 모델 하나는 여러 개의 메시로 이루어져 있다. */
/* 모델 = 메시 + 메시 + 메시... */
#include "Component.h"
#include "MeshBinary.h"

BEGIN(Engine)

/* 나무/드론/제트기처럼 뼈대 없이 통으로 그리는 모델.
   FBX 가 아니라 같은 이름의 .bin 을 읽는다(형식은 MeshBinary.h 참고).
   .bin 이 없으면 USE_ASSIMP_BAKE 가 1일 때만 FBX 로부터 한 번 구워서 만든다. */
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
	_uint						m_iNumMeshes = { 0 };
	vector< class CVIBuffer_Mesh*>			m_Meshes;

private:
	HRESULT Load_Binary(const string& strBinPath);

	/* "../bin/Models/Tree/Dead_Tree_3.FBX" -> "../bin/Models/Tree/Dead_Tree_3.bin" */
	static string Make_BinPath(const string& strModelFilePath);

public:
	static CMeshModel* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext, const string& strModelFilePath , _fmatrix PivotMatrix = XMMatrixIdentity(), _uint type = 0);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;


};

END
