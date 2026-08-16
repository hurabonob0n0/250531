#pragma once

/* 모델을 구성하는 하나의 덩어리(메시). 정점/인덱스 버퍼를 들고 있는다. */
#include "VIBuffer.h"
#include "MeshBinary.h"

BEGIN(Engine)

class CVIBuffer_Mesh final : public CVIBuffer
{
private:
	CVIBuffer_Mesh(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext);
	CVIBuffer_Mesh(CVIBuffer_Mesh& rhs);
	virtual ~CVIBuffer_Mesh() = default;

public:
	/* .bin 에서 읽어온 정점/인덱스를 그대로 올린다.
	   피벗행렬과 탄젠트는 구울 때 이미 반영돼 있어서 여기서 할 계산이 없다. */
	virtual HRESULT Initialize_Prototype(const VTXMESH* pVertices, _uint iVertexNum, const _ulong* pIndices, _uint iIndexNum);
	virtual HRESULT Initialize(void* pArg);

public:
	void Update();
	virtual HRESULT Render(int instanceNum = 1);

public:
	static CVIBuffer_Mesh* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext,
		const VTXMESH* pVertices, _uint iVertexNum, const _ulong* pIndices, _uint iIndexNum);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END
