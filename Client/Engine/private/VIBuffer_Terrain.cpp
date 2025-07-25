#include "VIBuffer_Terrain.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GameInstance.h"

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
    if (!imgData) return;

#pragma region VTXMesh

    m_NumVerticesPerRow = width;
    m_NumVerticesPerCol = height;

    m_VertexNum = width * height;
    m_VertexByteStride = sizeof(VTXMESH);
    m_VertexBufferByteSize = m_VertexNum * m_VertexByteStride;

    //m_vertices.resize(m_VertexNum);

    vector<VTXMESH> vecPoints;
    vecPoints.resize(m_VertexNum);

    // Vertex positions
    for (int z = 0; z < height; ++z)
    {
        for (int x = 0; x < width; ++x)
        {
            int idx = z * width + x;
            float y = (float)imgData[idx] * heightScale * 2.55f;

            vecPoints[idx].vPosition = XMFLOAT3(
                (x - width / 2) * cellSpacing, y, (z - height / 2) * cellSpacing);
            vecPoints[idx].vNormal = XMFLOAT3(0.f, 0.f, 0.f); // 임시 노멀값 (후처리 필요)
            vecPoints[idx].vTangent = XMFLOAT3(0.f, 0.f, 0.f);
            vecPoints[idx].vTexcoord = XMFLOAT2( x / ((float)width*2.f)+0.25f, z / ((float)height * 2.f) + 0.25f);
            //m_vertices[idx] = y;
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

            vSour = XMLoadFloat3(&vecPoints[iIndices[1]].vPosition) - XMLoadFloat3(&vecPoints[iIndices[0]].vPosition);
            vDest = XMLoadFloat3(&vecPoints[iIndices[2]].vPosition) - XMLoadFloat3(&vecPoints[iIndices[1]].vPosition);
            vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

            XMStoreFloat3(&vecPoints[iIndices[0]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&vecPoints[iIndices[0]].vNormal)) + vNormal);
            XMStoreFloat3(&vecPoints[iIndices[1]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&vecPoints[iIndices[1]].vNormal)) + vNormal);
            XMStoreFloat3(&vecPoints[iIndices[2]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&vecPoints[iIndices[2]].vNormal)) + vNormal);


            indices[iNumIndices++] = iIndices[0];
            indices[iNumIndices++] = iIndices[2];
            indices[iNumIndices++] = iIndices[3];

            vSour = XMLoadFloat3(&vecPoints[iIndices[2]].vPosition) - XMLoadFloat3(&vecPoints[iIndices[0]].vPosition);
            vDest = XMLoadFloat3(&vecPoints[iIndices[3]].vPosition) - XMLoadFloat3(&vecPoints[iIndices[2]].vPosition);
            vNormal = XMVector3Normalize(XMVector3Cross(vSour, vDest));

            XMStoreFloat3(&vecPoints[iIndices[0]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&vecPoints[iIndices[0]].vNormal)) + vNormal);
            XMStoreFloat3(&vecPoints[iIndices[2]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&vecPoints[iIndices[2]].vNormal)) + vNormal);
            XMStoreFloat3(&vecPoints[iIndices[3]].vNormal,
                XMVector3Normalize(XMLoadFloat3(&vecPoints[iIndices[3]].vNormal)) + vNormal);
        }
    }
    // 각 정점 Tangent 누적용 초기화
    for (int i = 0; i < vecPoints.size(); ++i)
        vecPoints[i].vTangent = XMFLOAT3(0.f, 0.f, 0.f);

    // 삼각형 순회
    for (size_t i = 0; i < m_NumVerticesPerCol - 1; i++)
    {
        for (size_t j = 0; j < m_NumVerticesPerRow - 1; j++)
        {
            UINT32 iIndex = i * m_NumVerticesPerRow + j;

            UINT32 iIndices[4] = {
                iIndex + m_NumVerticesPerRow,
                iIndex + m_NumVerticesPerRow + 1,
                iIndex + 1,
                iIndex
            };

            // ------ 삼각형 1 ------

            UINT32 i0 = iIndices[0];
            UINT32 i1 = iIndices[1];
            UINT32 i2 = iIndices[2];

            XMFLOAT3 p0 = vecPoints[i0].vPosition;
            XMFLOAT3 p1 = vecPoints[i1].vPosition;
            XMFLOAT3 p2 = vecPoints[i2].vPosition;

            XMFLOAT2 uv0 = vecPoints[i0].vTexcoord;
            XMFLOAT2 uv1 = vecPoints[i1].vTexcoord;
            XMFLOAT2 uv2 = vecPoints[i2].vTexcoord;

            XMVECTOR edge1 = XMLoadFloat3(&p1) - XMLoadFloat3(&p0);
            XMVECTOR edge2 = XMLoadFloat3(&p2) - XMLoadFloat3(&p0);

            float du1 = uv1.x - uv0.x;
            float dv1 = uv1.y - uv0.y;
            float du2 = uv2.x - uv0.x;
            float dv2 = uv2.y - uv0.y;

            float f = 1.0f / (du1 * dv2 - du2 * dv1 + 1e-6f);

            XMVECTOR tangent = f * (dv2 * edge1 - dv1 * edge2);

            vecPoints[i0].vTangent = Add3(vecPoints[i0].vTangent, tangent);
            vecPoints[i1].vTangent = Add3(vecPoints[i1].vTangent, tangent);
            vecPoints[i2].vTangent = Add3(vecPoints[i2].vTangent, tangent);

            // ------ 삼각형 2 ------

            i0 = iIndices[0];
            i1 = iIndices[2];
            i2 = iIndices[3];

            p0 = vecPoints[i0].vPosition;
            p1 = vecPoints[i1].vPosition;
            p2 = vecPoints[i2].vPosition;

            uv0 = vecPoints[i0].vTexcoord;
            uv1 = vecPoints[i1].vTexcoord;
            uv2 = vecPoints[i2].vTexcoord;

            edge1 = XMLoadFloat3(&p1) - XMLoadFloat3(&p0);
            edge2 = XMLoadFloat3(&p2) - XMLoadFloat3(&p0);

            du1 = uv1.x - uv0.x;
            dv1 = uv1.y - uv0.y;
            du2 = uv2.x - uv0.x;
            dv2 = uv2.y - uv0.y;

            f = 1.0f / (du1 * dv2 - du2 * dv1 + 1e-6f);

            tangent = f * (dv2 * edge1 - dv1 * edge2);

            vecPoints[i0].vTangent = Add3(vecPoints[i0].vTangent, tangent);
            vecPoints[i1].vTangent = Add3(vecPoints[i1].vTangent, tangent);
            vecPoints[i2].vTangent = Add3(vecPoints[i2].vTangent, tangent);
        }
    }

    // Tangent 정규화
    for (auto& vtx : vecPoints)
    {
        XMStoreFloat3(&vtx.vTangent, XMVector3Normalize(XMLoadFloat3(&vtx.vTangent)));
    }
    m_VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_Device,
        m_CommandList, vecPoints.data(), m_VertexBufferByteSize, &m_VertexBufferUploader);

    m_IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_Device,
        m_CommandList, indices.data(), m_IndexBufferByteSize, &m_IndexBufferUploader);

    Save_TerrainMesh_ToFile("../bin/Models/Terrain/TerrainVertices", vecPoints, indices);

    m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    CGameInstance::Get_Instance()->Execute_Flush_and_Reset();

    vecPoints.clear();
}

void CVIBuffer_Terrain::Make_Buffer(const char* filepath)
{
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open())
    {
        
        return;
    }

    // 정점 정보 읽기
    ifs.read(reinterpret_cast<char*>(&m_VertexNum), sizeof(UINT));
    ifs.read(reinterpret_cast<char*>(&m_VertexByteStride), sizeof(UINT));
    ifs.read(reinterpret_cast<char*>(&m_VertexBufferByteSize), sizeof(UINT));

    std::vector<VTXMESH> vertices(m_VertexNum);
    ifs.read(reinterpret_cast<char*>(vertices.data()), m_VertexBufferByteSize);

    // 인덱스 정보 읽기
    ifs.read(reinterpret_cast<char*>(&m_IndexNum), sizeof(UINT));
    ifs.read(reinterpret_cast<char*>(&m_IndexFormat), sizeof(DXGI_FORMAT));
    ifs.read(reinterpret_cast<char*>(&m_IndexBufferByteSize), sizeof(UINT));

    std::vector<UINT32> indices(m_IndexNum);
    ifs.read(reinterpret_cast<char*>(indices.data()), m_IndexBufferByteSize);

    ifs.close();

    // 버퍼 생성
    m_VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_Device,
        m_CommandList, vertices.data(), m_VertexBufferByteSize, &m_VertexBufferUploader);

    m_IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_Device,
        m_CommandList, indices.data(), m_IndexBufferByteSize, &m_IndexBufferUploader);

    m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    CGameInstance::Get_Instance()->Execute_Flush_and_Reset();

    vertices.clear();
    indices.clear();
}

XMFLOAT3 CVIBuffer_Terrain::Add3(const XMFLOAT3& a, XMVECTOR b)
{
    XMVECTOR va = XMLoadFloat3(&a);
    va += b;
    XMFLOAT3 result;
    XMStoreFloat3(&result, va);
    return result;
}


void CVIBuffer_Terrain::Save_TerrainMesh_ToFile(const std::string& filename,
    const std::vector<VTXMESH>& vertices,
    const std::vector<UINT32>& indices)
{
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs.is_open())
    {
        return;
    }

    ofs.write(reinterpret_cast<const char*>(&m_VertexNum), sizeof(UINT));
    ofs.write(reinterpret_cast<const char*>(&m_VertexByteStride), sizeof(UINT));
    ofs.write(reinterpret_cast<const char*>(&m_VertexBufferByteSize), sizeof(UINT));
    ofs.write(reinterpret_cast<const char*>(vertices.data()), m_VertexBufferByteSize);


    ofs.write(reinterpret_cast<const char*>(&m_IndexNum), sizeof(UINT));
    ofs.write(reinterpret_cast<const char*>(&m_IndexFormat), sizeof(DXGI_FORMAT));
    ofs.write(reinterpret_cast<const char*>(&m_IndexBufferByteSize), sizeof(UINT));
    ofs.write(reinterpret_cast<const char*>(indices.data()), m_IndexBufferByteSize);

    ofs.close();

}

HRESULT CVIBuffer_Terrain::Render()
{
    m_CommandList->IASetVertexBuffers(0, 1, &VertexBufferView());
    m_CommandList->IASetIndexBuffer(&IndexBufferView());
    m_CommandList->IASetPrimitiveTopology(m_PrimitiveType);
    m_CommandList->DrawIndexedInstanced(m_IndexNum, 1, 0, 0, 0);
    return S_OK;
}

CVIBuffer_Terrain* CVIBuffer_Terrain::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, const char* pHeightmapPath)
{
    CVIBuffer_Terrain* pInstance = new CVIBuffer_Terrain(pDevice, pCommandList);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        return nullptr;
    }
    pInstance->Make_Buffer(pHeightmapPath);
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


