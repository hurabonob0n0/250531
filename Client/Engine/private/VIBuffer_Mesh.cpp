#include "VIBuffer_Mesh.h"
#include "Bone.h"
#include "Shader.h"
#include "GameInstance.h"

CVIBuffer_Mesh::CVIBuffer_Mesh(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Mesh::CVIBuffer_Mesh(CVIBuffer_Mesh & rhs)
	: CVIBuffer(rhs)
{
}

HRESULT CVIBuffer_Mesh::Initialize_Prototype(const aiMesh * pAIMesh)
{
	m_VertexNum = pAIMesh->mNumVertices;
	m_IndexNum = pAIMesh->mNumFaces * 3;
	m_IndexFormat = DXGI_FORMAT_R32_UINT;
	m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_IndexBufferByteSize = m_IndexNum * sizeof(std::uint32_t);

#pragma region VERTEX_BUFFER

	Ready_Mesh(pAIMesh);
	
#pragma endregion


#pragma region INDEX_BUFFER
	_ulong* pIndices = new _ulong[m_IndexNum];
	ZeroMemory(pIndices, m_IndexBufferByteSize);

	_uint		iNumIndices = 0;

	for (size_t i = 0; i < pAIMesh->mNumFaces; ++i)
	{
		aiFace	AIFace = pAIMesh->mFaces[i];

		pIndices[iNumIndices++] = AIFace.mIndices[0];
		pIndices[iNumIndices++] = AIFace.mIndices[1];
		pIndices[iNumIndices++] = AIFace.mIndices[2];
	}

	__super::Create_Buffer(&m_IndexBufferGPU, &m_IndexBufferUploader, pIndices, m_IndexBufferByteSize);

	delete[] pIndices;
	pIndices = nullptr;
#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Mesh::Initialize(void * pArg)
{
	return S_OK;
}

void CVIBuffer_Mesh::Update()
{
}

HRESULT CVIBuffer_Mesh::Render()
{
	__super::Render();
	return S_OK;
}


HRESULT CVIBuffer_Mesh::Ready_Mesh(const aiMesh * pAIMesh)
{
	m_VertexByteStride = sizeof(VTXMESH);
	m_VertexBufferByteSize = m_VertexByteStride * m_VertexNum;

	VTXMESH* pVertices = new VTXMESH[m_VertexNum];
	ZeroMemory(pVertices, sizeof(VTXMESH) * m_VertexNum);

	for (size_t i = 0; i < m_VertexNum; i++)
	{
		memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vPosition, XMLoadFloat3(&pVertices[i].vPosition));

		memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(_float3));
		XMStoreFloat3(&pVertices[i].vNormal, XMLoadFloat3(&pVertices[i].vNormal));

		memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
		memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(_float3));
	}

	__super::Create_Buffer(&m_VertexBufferGPU, &m_VertexBufferUploader, pVertices, m_VertexBufferByteSize);

	delete[] pVertices;
	pVertices = nullptr;

	return S_OK;
}

CVIBuffer_Mesh* CVIBuffer_Mesh::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pContext, const aiMesh* pAIMesh)
{
	CVIBuffer_Mesh* pInstance = new CVIBuffer_Mesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pAIMesh)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Mesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CVIBuffer_Mesh::Clone(void * pArg)
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
