#include "VIBuffer_Quad.h"

CVIBuffer_Quad::CVIBuffer_Quad(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
    : CVIBuffer(pDevice, pCommandList)
{
}

CVIBuffer_Quad::CVIBuffer_Quad(CVIBuffer_Quad& rhs)
    : CVIBuffer(rhs)
{
}

HRESULT CVIBuffer_Quad::Initialize_Prototype()
{
    //m_VertexNum = 4;
    //m_VertexByteStride = sizeof(VTXMESH);
    //m_VertexBufferByteSize = m_VertexByteStride * m_VertexNum;

    //VTXMESH vertices[4];

    //vertices[0].vPosition = XMFLOAT3(-0.5f, 0.5f, 0.0f);   // LT
    //vertices[0].vNormal = XMFLOAT3(0.f, 0.f, -1.f);
    //vertices[0].vTexcoord = XMFLOAT2(0.0f, 0.0f);
    //vertices[0].vTangent = XMFLOAT3(1.f, 0.f, 0.f);

    //vertices[1].vPosition = XMFLOAT3(0.5f, 0.5f, 0.0f);    // RT
    //vertices[1].vNormal = XMFLOAT3(0.f, 0.f, -1.f);
    //vertices[1].vTexcoord = XMFLOAT2(1.0f, 0.0f);
    //vertices[1].vTangent = XMFLOAT3(1.f, 0.f, 0.f);

    //vertices[2].vPosition = XMFLOAT3(0.5f, -0.5f, 0.0f);   // RB
    //vertices[2].vNormal = XMFLOAT3(0.f, 0.f, -1.f);
    //vertices[2].vTexcoord = XMFLOAT2(1.0f, 1.0f);
    //vertices[2].vTangent = XMFLOAT3(1.f, 0.f, 0.f);

    //vertices[3].vPosition = XMFLOAT3(-0.5f, -0.5f, 0.0f);  // LB
    //vertices[3].vNormal = XMFLOAT3(0.f, 0.f, -1.f);
    //vertices[3].vTexcoord = XMFLOAT2(0.0f, 1.0f);
    //vertices[3].vTangent = XMFLOAT3(1.f, 0.f, 0.f);

    //std::uint16_t indices[] = {
    //    0, 1, 2,
    //    0, 2, 3
    //};

    //m_IndexNum = _countof(indices);
    //m_IndexFormat = DXGI_FORMAT_R16_UINT;
    //m_IndexBufferByteSize = sizeof(indices);

    //Create_Buffer(&m_VertexBufferGPU, &m_VertexBufferUploader, vertices, m_VertexBufferByteSize);
    //Create_Buffer(&m_IndexBufferGPU, &m_IndexBufferUploader, indices, m_IndexBufferByteSize);

    //m_PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    const UINT gridCountX = 8;
    const UINT gridCountY = 8;
    const UINT vertexCountX = gridCountX + 1;
    const UINT vertexCountY = gridCountY + 1;

    m_VertexNum = vertexCountX * vertexCountY;
    m_VertexByteStride = sizeof(VTXMESH);
    m_VertexBufferByteSize = m_VertexByteStride * m_VertexNum;

    std::vector<VTXMESH> vertices(m_VertexNum);

    for (UINT y = 0; y < vertexCountY; ++y)
    {
        float fy = 1.0f - (float)y / gridCountY; // 위에서 아래로
        for (UINT x = 0; x < vertexCountX; ++x)
        {
            float fx = (float)x / gridCountX;

            UINT idx = y * vertexCountX + x;

            vertices[idx].vPosition = XMFLOAT3((fx - 0.5f), (fy - 0.5f), 0.0f);
            vertices[idx].vNormal = XMFLOAT3(0.f, 0.f, -1.f);
            vertices[idx].vTexcoord = XMFLOAT2(fx, 1.0f - fy); // 텍스처 상하 반전
            vertices[idx].vTangent = XMFLOAT3(1.f, 0.f, 0.f);
        }
    }

    std::vector<std::uint16_t> indices;
    indices.reserve(gridCountX * gridCountY * 6); // 2 tri per quad

    for (UINT y = 0; y < gridCountY; ++y)
    {
        for (UINT x = 0; x < gridCountX; ++x)
        {
            UINT i0 = y * vertexCountX + x;
            UINT i1 = i0 + 1;
            UINT i2 = i0 + vertexCountX;
            UINT i3 = i2 + 1;

            // 첫 삼각형
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i3);

            // 두 번째 삼각형
            indices.push_back(i0);
            indices.push_back(i3);
            indices.push_back(i2);
        }
    }

    m_IndexNum = static_cast<UINT>(indices.size());
    m_IndexFormat = DXGI_FORMAT_R16_UINT;
    m_IndexBufferByteSize = sizeof(std::uint16_t) * m_IndexNum;

    Create_Buffer(&m_VertexBufferGPU, &m_VertexBufferUploader, vertices.data(), m_VertexBufferByteSize);
    Create_Buffer(&m_IndexBufferGPU, &m_IndexBufferUploader, indices.data(), m_IndexBufferByteSize);

    m_PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return S_OK;
}

HRESULT CVIBuffer_Quad::Initialize(void* pArg)
{
    return S_OK;
}

HRESULT CVIBuffer_Quad::Render(int instanceNum)
{
    return __super::Render(instanceNum);
}

CVIBuffer_Quad* CVIBuffer_Quad::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
{
    CVIBuffer_Quad* pInstance = new CVIBuffer_Quad(pDevice, pCommandList);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CVIBuffer_Quad");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CVIBuffer_Quad::Clone(void* pArg)
{
    CVIBuffer_Quad* pInstance = new CVIBuffer_Quad(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Clone : CVIBuffer_Quad");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CVIBuffer_Quad::Free()
{
    __super::Free();
}