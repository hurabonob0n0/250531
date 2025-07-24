#pragma once
#include "VIBuffer.h"
#include <vector>

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Terrain final : public CVIBuffer
{
protected:
    CVIBuffer_Terrain(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
    CVIBuffer_Terrain(CVIBuffer_Terrain& rhs);
    virtual ~CVIBuffer_Terrain() = default;

public:
    float Get_Terrain_Heights(float x, float z);

public:
    HRESULT Initialize_Prototype();
    HRESULT Initialize(void* pArg) override;

public:
    void Make_Buffer(const char* pHeightmapPath, float heightScale, float cellSpacing);
    void Make_Buffer(const char* filepath);
    void Read_Map();
    XMFLOAT3 Add3(const XMFLOAT3& a, XMVECTOR b);

public:
    void Save_TerrainMesh_ToFile(const std::string& filename, const std::vector<VTXMESH>& vertices, const std::vector<UINT32>& indices);
    HRESULT Render() override;

    inline int Get_Index(int x, int z) const {
        return z * 4096 + x;
    }

private:
    std::vector<VTXPOSNOR> m_vertices;
    vector<float> m_heightMap;

public:
    static CVIBuffer_Terrain* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, const char* pHeightmapPath);
    CComponent* Clone(void* pArg) override;
    virtual void Free() override;

private:
    int m_NumVerticesPerRow = 0;
    int m_NumVerticesPerCol = 0;
};

END

