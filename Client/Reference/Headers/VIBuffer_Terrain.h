#pragma once
#include "VIBuffer.h"
#include "Engine_Config.h"
#include <vector>
#include <memory>

BEGIN(Engine)

/*===========================================================================
    청크 + LOD 지형

    예전에는 4096x4096 정점(1670만개, VB 740MB / IB 400MB)을 통째로 한 번에
    그렸다. 삼각형 3350만개를 그림자패스까지 두 번 그리니 노트북에서 돌 리가 없었다.

    지금은
      - 정점을 TERRAIN_VERTEX_STEP 간격으로만 만들고
      - 청크(TERRAIN_CHUNK_CELLS 칸) 단위로 잘라서
      - 절두체 밖 청크는 아예 그리지 않고
      - 카메라에서 먼 청크는 인덱스를 띄엄띄엄 읽는 LOD 로 그린다.

    정점버퍼는 청크 순서대로 이어붙인 딱 하나뿐이고, 인덱스버퍼도 하나다.
    청크마다 BaseVertexLocation 만 바꿔서 같은 LOD 인덱스를 재사용한다.
===========================================================================*/
class ENGINE_DLL CVIBuffer_Terrain final : public CVIBuffer
{
public:
    struct TERRAINCHUNK
    {
        _uint       BaseVertex;     /* 전체 정점버퍼에서 이 청크가 시작되는 정점 번호 */
        XMFLOAT3    vCenter;        /* AABB 중심 */
        XMFLOAT3    vExtents;       /* AABB 반크기 */
    };

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
    /* pHeightFilePath : Terrain4096Map.bin (정점당 float3) 또는 Terrain4096.hgt (높이만) */
    void Make_Buffer(const char* pHeightFilePath);
    HRESULT Render(int instanceNum = 1) override;

public:
    /* 디버그용 통계 - 이번 프레임에 실제로 그린 양 */
    _uint Get_DrawnChunks() const { return m_DrawnChunks; }
    _uint Get_DrawnTriangles() const { return m_DrawnTriangles; }

    inline int Get_Index(int x, int z) const {
        return z * TERRAIN_MAP_SIZE + x;
    }

private:
    bool    Load_HeightMap(const char* pHeightFilePath);
    void    Build_Vertices(std::vector<VTXMESH>& Vertices);
    void    Build_Indices(std::vector<_ushort>& Indices);
    void    Extract_FrustumPlanes(_fmatrix ViewProj);
    bool    Is_Chunk_Visible(const TERRAINCHUNK& Chunk) const;
    _uint   Pick_LOD(const TERRAINCHUNK& Chunk, const XMFLOAT3& vEye) const;
    float   Get_Sample_Height(int iSampleX, int iSampleZ) const;

private:
    /* 높이 원본(4096x4096). 게임로직(Get_Terrain_Heights)이 쓰므로 원본 해상도를 유지한다.
       프로토타입과 복제본이 같은 배열을 공유한다. */
    std::shared_ptr<std::vector<float>> m_pHeightMap;

    std::vector<TERRAINCHUNK>   m_Chunks;
    _uint                       m_ChunksPerSide = { 0 };
    _uint                       m_VerticesPerChunk = { 0 };

    /* LOD 단계별로 인덱스버퍼 안의 구간. 격자와 치마(변 4개)를 따로 잡아둔다.
       치마는 '이웃 청크와 LOD 가 다른 변'에만 그린다 - 같은 LOD 끼리는 틈이 없으므로
       치마를 그리면 경계마다 벽이 서서 바닥에 격자 무늬 선으로 보인다. */
    _uint                       m_LODGridStart[TERRAIN_LOD_COUNT] = {};
    _uint                       m_LODGridCount[TERRAIN_LOD_COUNT] = {};
    _uint                       m_LODSkirtStart[TERRAIN_LOD_COUNT][4] = {};
    _uint                       m_LODSkirtCount[TERRAIN_LOD_COUNT][4] = {};

    /* Render 에서 매 프레임 재사용하는 작업버퍼(청크별 LOD) */
    std::vector<_uint>          m_ChunkLODs;

    XMFLOAT4                    m_FrustumPlanes[6] = {};
    bool                        m_isCullingValid = { false };

    _uint                       m_DrawnChunks = { 0 };
    _uint                       m_DrawnTriangles = { 0 };

public:
    static CVIBuffer_Terrain* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, const char* pHeightFilePath);
    CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

END
