#include "VIBuffer_Mesh.h"
#include "Bone.h"
#include "Shader.h"
#include "GameInstance.h"

CVIBuffer_Mesh::CVIBuffer_Mesh(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Mesh::CVIBuffer_Mesh(CVIBuffer_Mesh& rhs)
	: CVIBuffer(rhs)
{
}

HRESULT CVIBuffer_Mesh::Initialize_Prototype(const VTXMESH* pVertices, _uint iVertexNum, const _ulong* pIndices, _uint iIndexNum)
{
	if (nullptr == pVertices || 0 == iVertexNum || nullptr == pIndices || 0 == iIndexNum)
		return E_FAIL;

	m_VertexByteStride = sizeof(VTXMESH);
	m_VertexNum = iVertexNum;
	m_VertexBufferByteSize = m_VertexNum * m_VertexByteStride;

	m_IndexFormat = DXGI_FORMAT_R32_UINT;
	m_IndexNum = iIndexNum;
	m_IndexBufferByteSize = m_IndexNum * sizeof(_ulong);

	m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	__super::Create_Buffer(&m_VertexBufferGPU, &m_VertexBufferUploader, pVertices, m_VertexBufferByteSize);
	__super::Create_Buffer(&m_IndexBufferGPU, &m_IndexBufferUploader, pIndices, m_IndexBufferByteSize);

	return S_OK;
}

HRESULT CVIBuffer_Mesh::Initialize(void* pArg)
{
	return S_OK;
}

void CVIBuffer_Mesh::Update()
{
}

HRESULT CVIBuffer_Mesh::Render(int instanceNum)
{
	__super::Render(instanceNum);
	return S_OK;
}

CVIBuffer_Mesh* CVIBuffer_Mesh::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext,
	const VTXMESH* pVertices, _uint iVertexNum, const _ulong* pIndices, _uint iIndexNum)
{
	CVIBuffer_Mesh* pInstance = new CVIBuffer_Mesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pVertices, iVertexNum, pIndices, iIndexNum)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Mesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Mesh::Clone(void* pArg)
{
	CVIBuffer_Mesh* pInstance = new CVIBuffer_Mesh(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_Mesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Mesh::Free()
{
	__super::Free();
}
