#pragma once
#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Geos : public CVIBuffer
{
public:
	enum BASIC_SUBMESHES { BS_BOX, BS_GRID, BS_SPHERE, BS_CYLINDER, BS_END };

protected:
	CVIBuffer_Geos(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	CVIBuffer_Geos(CVIBuffer_Geos& rhs);
	virtual~CVIBuffer_Geos() = default;

public:
	HRESULT Render() override;

protected:
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;

protected:
	unordered_map<BASIC_SUBMESHES, SubmeshGeometry>	m_SubMeshInfos;
	BASIC_SUBMESHES									m_RenderedSubMeshName;
	SubmeshGeometry									m_RenderedSubMesh;

public:
	static CVIBuffer_Geos* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

END