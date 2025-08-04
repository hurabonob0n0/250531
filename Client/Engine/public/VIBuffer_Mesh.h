#pragma once

/* 모델을 구성하는 하나의 파츠. */
/* 정점과 인덱스를 구성하기위해 디자인된 클래스다. */
#include "VIBuffer.h"
#include "Model.h"


BEGIN(Engine)

class CVIBuffer_Mesh final : public CVIBuffer
{
private:
	CVIBuffer_Mesh(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext);
	CVIBuffer_Mesh(CVIBuffer_Mesh& rhs);
	virtual ~CVIBuffer_Mesh() = default;

public:
	virtual HRESULT Initialize_Prototype(const aiMesh* pAIMesh, _fmatrix PivotMatrix = XMMatrixIdentity(), _uint type = 0);
	virtual HRESULT Initialize(void* pArg);

public:
	void Update();
	virtual HRESULT Render();

private:
	HRESULT Ready_Mesh(const aiMesh* pAIMesh, _fmatrix PivotMatrix = XMMatrixIdentity(), _uint type = 0);


public:
	static CVIBuffer_Mesh* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext,  const aiMesh* pAIMesh, _fmatrix PivotMatrix = XMMatrixIdentity(), _uint type = 0);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END