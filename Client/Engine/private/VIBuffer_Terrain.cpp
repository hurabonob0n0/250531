#include "VIBuffer_Terrain.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NUMVERTERTICES 4096

CVIBuffer_Terrain::CVIBuffer_Terrain(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) : CVIBuffer(pDevice, pCommandList) {}

CVIBuffer_Terrain::CVIBuffer_Terrain(CVIBuffer_Terrain& rhs): CVIBuffer(rhs), m_NumVerticesPerRow(rhs.m_NumVerticesPerRow), m_NumVerticesPerCol(rhs.m_NumVerticesPerCol) 
{}

float CVIBuffer_Terrain::Get_Terrain_Heights(float x, float z)
{
    int LX = int(x + 4096.f);
    int DZ = int(z + 4096.f);

    if (x < -4096 || z < -4096 || x > 4096 || z > 4096)
        return 0.f;

    _vector Positions[4];

    Positions[0] = XMLoadFloat3( &m_vertices[(DZ + 1) * 8192 + LX].vPosition);
    Positions[1] = XMLoadFloat3( &m_vertices[(DZ + 1) * 8192 + LX + 1].vPosition);
    Positions[2] = XMLoadFloat3( &m_vertices[DZ * 8192 + LX + 1].vPosition);
    Positions[3] = XMLoadFloat3( &m_vertices[DZ * 8192 + LX].vPosition);

    float DeltaX = x - float(LX) + 4096.f;
    float DeltaZ = z - float(DZ) + 4096.f;

    _vector PlaneNormal;

    if (DeltaX + DeltaZ <= 1)
        PlaneNormal = XMPlaneFromPoints(Positions[0], Positions[2], Positions[3]);

    if (DeltaX + DeltaZ > 1)
        PlaneNormal = XMPlaneFromPoints(Positions[0], Positions[1], Positions[2]);

    return -(XMVectorGetX(PlaneNormal) * x + XMVectorGetZ(PlaneNormal) * z + XMVectorGetW(PlaneNormal)) / XMVectorGetY(PlaneNormal) + 0.5f;
}

HRESULT CVIBuffer_Terrain::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CVIBuffer_Terrain::Initialize(void* pArg)
{
    return S_OK;
}

void CVIBuffer_Terrain::Make_Buffer(const char* pHeightmapPath, float heightScale, float cellSpacing)
{
    int width, height, channels;
    unsigned char* imgData = stbi_load(pHeightmapPath, &width, &height, &channels, 1);
    if (!imgData) return ;

    m_NumVerticesPerRow = width;
    m_NumVerticesPerCol = height;

    m_VertexNum = width * height;
    m_VertexByteStride = sizeof(VTXPOSNOR);
    m_VertexBufferByteSize = m_VertexNum * m_VertexByteStride;

    m_vertices.resize(m_VertexNum);

    // Vertex positions
    for (int z = 0; z < height; ++z)
    {
        for (int x = 0; x < width; ++x)
        {
            int idx = z * width + x;
            float y = (float)imgData[idx] * heightScale * 2.55f;

            m_vertices[idx].vPosition = XMFLOAT3(
                (x - width / 2) * cellSpacing, y, (z - height / 2) * cellSpacing);
            m_vertices[idx].vNormal = XMFLOAT3(0.f, 0.f, 0.f); // 임시 노멀값 (후처리 필요)
        }
    }

    stbi_image_free(imgData);

    // Index 생성
    m_IndexNum = (width - 1) * (height - 1) * 6;
    m_IndexFormat = DXGI_FORMAT_R32_UINT;
    m_IndexBufferByteSize = m_IndexNum * sizeof(UINT32);

    std::vector<UINT32> indices(m_IndexNum);
    
    UINT32		iNumIndices = 0;

    for (size_t i = 0; i < m_NumVerticesPerCol - 1; i++)
    {
        for (size_t j = 0; j < m_NumVerticesPerRow - 1; j++)
        {
            UINT32		iIndex = i * m_NumVerticesPerRow + j;

            UINT32		iIndices[4] = {
                iIndex + m_NumVerticesPerRow,
                iIndex + m_NumVerticesPerRow + 1,
                iIndex + 1,
                iIndex
            };

            XMVECTOR		vSour, vDest, vNormal;

            indices[iNumIndices++] = iIndices[0];
            indices[iNumIndices++] = iIndices[1];
            indices[iNumIndices++] = iIndices[2];

            vSour = XMLoadFloat3(&m_vertices[iIndices[1]].vPosition) - XMLoadFloat3(&m_vertices[iIndices[0]].vPosition);
            vDest = XMLoadFloat3(&m_vertices[iIndices[2]].vPosition) - XMLoadFloat3(&m_vertices[iIndices[1]].vPosition);
            vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

            XMStoreFloat3(&m_vertices[iIndices[0]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&m_vertices[iIndices[0]].vNormal)) + vNormal);
            XMStoreFloat3(&m_vertices[iIndices[1]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&m_vertices[iIndices[1]].vNormal)) + vNormal);
            XMStoreFloat3(&m_vertices[iIndices[2]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&m_vertices[iIndices[2]].vNormal)) + vNormal);


            indices[iNumIndices++] = iIndices[0];
            indices[iNumIndices++] = iIndices[2];
            indices[iNumIndices++] = iIndices[3];

            vSour = XMLoadFloat3(&m_vertices[iIndices[2]].vPosition) - XMLoadFloat3(&m_vertices[iIndices[0]].vPosition);
            vDest = XMLoadFloat3(&m_vertices[iIndices[3]].vPosition) - XMLoadFloat3(&m_vertices[iIndices[2]].vPosition);
            vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

            XMStoreFloat3(&m_vertices[iIndices[0]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&m_vertices[iIndices[0]].vNormal)) + vNormal);
            XMStoreFloat3(&m_vertices[iIndices[2]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&m_vertices[iIndices[2]].vNormal)) + vNormal);
            XMStoreFloat3(&m_vertices[iIndices[3]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&m_vertices[iIndices[3]].vNormal)) + vNormal);
        }
    }




    m_VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_Device,
        m_CommandList, m_vertices.data(), m_VertexBufferByteSize, &m_VertexBufferUploader);

    m_IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_Device,
        m_CommandList, indices.data(), m_IndexBufferByteSize, &m_IndexBufferUploader);

    m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

HRESULT CVIBuffer_Terrain::Render()
{
    m_CommandList->IASetVertexBuffers(0, 1, &VertexBufferView());
    m_CommandList->IASetIndexBuffer(&IndexBufferView());
    m_CommandList->IASetPrimitiveTopology(m_PrimitiveType);
    m_CommandList->DrawIndexedInstanced(m_IndexNum, 1, 0, 0, 0);
    return S_OK;
}

CVIBuffer_Terrain* CVIBuffer_Terrain::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, const char* pHeightmapPath, float heightScale, float cellSpacing)
{
    CVIBuffer_Terrain* pInstance = new CVIBuffer_Terrain(pDevice, pCommandList);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        return nullptr;
    }
    pInstance->Make_Buffer(pHeightmapPath, heightScale, cellSpacing);
    return pInstance;
}

CComponent* CVIBuffer_Terrain::Clone(void* pArg)
{
    CVIBuffer_Terrain* pInstance = new CVIBuffer_Terrain(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        Safe_Release(pInstance);
        return nullptr;
    }
    return pInstance;
}

void CVIBuffer_Terrain::Free()
{
    CVIBuffer::Free();
}


