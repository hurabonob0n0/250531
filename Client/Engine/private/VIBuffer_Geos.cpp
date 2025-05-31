#include "VIBuffer_Geos.h"

CVIBuffer_Geos::CVIBuffer_Geos(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) : CVIBuffer(pDevice, pCommandList)
{
}

CVIBuffer_Geos::CVIBuffer_Geos(CVIBuffer_Geos& rhs) : CVIBuffer(rhs)
{
	m_SubMeshInfos.reserve((int)BS_END);
	m_SubMeshInfos = rhs.m_SubMeshInfos;
}

HRESULT CVIBuffer_Geos::Render()
{
	m_CommandList->IASetVertexBuffers(0, 1, &VertexBufferView());
	m_CommandList->IASetIndexBuffer(&IndexBufferView());
	m_CommandList->IASetPrimitiveTopology(m_PrimitiveType);

	m_CommandList->DrawIndexedInstanced(m_RenderedSubMesh.IndexCount, 1, m_RenderedSubMesh.StartIndexLocation, m_RenderedSubMesh.BaseVertexLocation, 0);

	return S_OK;
}

HRESULT CVIBuffer_Geos::Initialize_Prototype()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.f, 1.f, 1.f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.5f, 1.0f, 20, 20);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

	// Define the SubmeshGeometry that cover different 
	// regions of the vertex/index buffers.

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	m_VertexNum =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	std::vector<VTXMESH> vertices(m_VertexNum);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].vPosition = box.Vertices[i].Position;
		vertices[k].vNormal = box.Vertices[i].Normal;
		vertices[k].vTexcoord = box.Vertices[i].TexC;
		vertices[k].vTangent = box.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].vPosition = grid.Vertices[i].Position;
		vertices[k].vNormal = grid.Vertices[i].Normal;
		vertices[k].vTexcoord = grid.Vertices[i].TexC;
		vertices[k].vTangent = grid.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].vPosition = sphere.Vertices[i].Position;
		vertices[k].vNormal = sphere.Vertices[i].Normal;
		vertices[k].vTexcoord = sphere.Vertices[i].TexC;
		vertices[k].vTangent = sphere.Vertices[i].TangentU;
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].vPosition = cylinder.Vertices[i].Position;
		vertices[k].vNormal = cylinder.Vertices[i].Normal;
		vertices[k].vTexcoord = cylinder.Vertices[i].TexC;
		vertices[k].vTangent = cylinder.Vertices[i].TangentU;
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

	m_IndexNum = indices.size();
	m_IndexFormat = DXGI_FORMAT_R16_UINT;

	const UINT vbByteSize = m_VertexNum * sizeof(VTXMESH);
	const UINT ibByteSize = m_IndexNum * sizeof(std::uint16_t);

	__super::Create_Buffer(&m_VertexBufferGPU, &m_VertexBufferUploader, vertices.data(), vbByteSize);

	__super::Create_Buffer(&m_IndexBufferGPU, &m_IndexBufferUploader, indices.data(), ibByteSize);

	m_VertexByteStride = sizeof(VTXMESH);
	m_VertexBufferByteSize = vbByteSize;
	m_IndexFormat = DXGI_FORMAT_R16_UINT;
	m_IndexBufferByteSize = ibByteSize;

	m_SubMeshInfos[BASIC_SUBMESHES::BS_BOX] = boxSubmesh;
	m_SubMeshInfos[BASIC_SUBMESHES::BS_GRID] = gridSubmesh;
	m_SubMeshInfos[BASIC_SUBMESHES::BS_SPHERE] = sphereSubmesh;
	m_SubMeshInfos[BASIC_SUBMESHES::BS_CYLINDER] = cylinderSubmesh;

	m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_RenderedSubMeshName = BASIC_SUBMESHES::BS_BOX;
	m_RenderedSubMesh = m_SubMeshInfos[m_RenderedSubMeshName];

	m_IndexNum = m_SubMeshInfos[m_RenderedSubMeshName].IndexCount;

	return S_OK;
}

HRESULT CVIBuffer_Geos::Initialize(void* pArg)
{
	if (pArg == nullptr)
	{
		m_RenderedSubMeshName = BASIC_SUBMESHES::BS_BOX;
		m_RenderedSubMesh = m_SubMeshInfos[m_RenderedSubMeshName];
		return S_OK;
	}

	m_RenderedSubMeshName = *(BASIC_SUBMESHES*)pArg;
	m_RenderedSubMesh = m_SubMeshInfos[m_RenderedSubMeshName];

	return S_OK;
}

CVIBuffer_Geos* CVIBuffer_Geos::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
{
	CVIBuffer_Geos* pInstance = new CVIBuffer_Geos(pDevice, pCommandList);
	if (FAILED(pInstance->Initialize_Prototype())) {
		MSG_BOX("Failed to Create : CVIBuffer_Geo");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CVIBuffer_Geos::Clone(void* pArg)
{
	CVIBuffer_Geos* pInstance = new CVIBuffer_Geos(*this);

	if (FAILED(pInstance->Initialize(pArg))) {
		MSG_BOX("Failed to Cloned : CVIBuffer_Geo");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Geos::Free()
{
	m_SubMeshInfos.clear();

	__super::Free();
}