#include "VIBuffer_Terrain.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GameInstance.h"
#include <float.h>

namespace
{
    /* 파생 상수들 - Engine_Config.h 의 값에서 계산된다. */
    constexpr int   MAP = TERRAIN_MAP_SIZE;                          /* 4096 */
    constexpr int   STEP = TERRAIN_VERTEX_STEP;                      /* 샘플 간격 */
    constexpr int   CELLS = TERRAIN_CHUNK_CELLS;                     /* 청크 한 변의 셀 수 */
    constexpr int   VERTS_PER_SIDE = CELLS + 1;                      /* 청크 한 변의 정점 수 */
    constexpr int   GRID_VERTS = VERTS_PER_SIDE * VERTS_PER_SIDE;    /* 청크의 격자 정점 수 */
    constexpr int   SKIRT_VERTS = 4 * VERTS_PER_SIDE;                /* 청크의 치마 정점 수 */
    constexpr int   CHUNK_VERTS = GRID_VERTS + SKIRT_VERTS;
    constexpr int   CHUNK_WORLD = CELLS * STEP;                      /* 청크 한 변의 월드 크기 */
    constexpr int   CHUNKS_PER_SIDE = MAP / CHUNK_WORLD;
    constexpr float HALF = MAP * 0.5f;

    /* 치마 정점의 시작 오프셋. 변(edge) 0:z최소 1:z최대 2:x최소 3:x최대 */
    inline int Skirt_Local(int iEdge, int i) { return GRID_VERTS + iEdge * VERTS_PER_SIDE + i; }
    inline int Grid_Local(int x, int z) { return z * VERTS_PER_SIDE + x; }

    /* 치마 정점 하나가 베껴올 격자 정점 */
    inline int Skirt_Source(int iEdge, int i)
    {
        switch (iEdge)
        {
        case 0: return Grid_Local(i, 0);
        case 1: return Grid_Local(i, CELLS);
        case 2: return Grid_Local(0, i);
        default: return Grid_Local(CELLS, i);
        }
    }
}

/* 컴파일 타임에 실수를 잡는다. */
static_assert(MAP % CHUNK_WORLD == 0, "TERRAIN_MAP_SIZE 가 청크 크기로 나누어떨어져야 한다.");
static_assert(CHUNK_VERTS <= 65535, "청크 정점이 65535 를 넘으면 16비트 인덱스를 쓸 수 없다.");
static_assert(CELLS % (1 << (TERRAIN_LOD_COUNT - 1)) == 0, "가장 거친 LOD 간격으로 셀 수가 나누어떨어져야 한다.");

CVIBuffer_Terrain::CVIBuffer_Terrain(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) : CVIBuffer(pDevice, pCommandList) {}

CVIBuffer_Terrain::CVIBuffer_Terrain(CVIBuffer_Terrain& rhs)
    : CVIBuffer(rhs)
    , m_pHeightMap(rhs.m_pHeightMap)            /* 높이 배열은 공유한다(67MB 를 두 벌 들고 있을 이유가 없다) */
    , m_Chunks(rhs.m_Chunks)
    , m_ChunksPerSide(rhs.m_ChunksPerSide)
    , m_VerticesPerChunk(rhs.m_VerticesPerChunk)
{
    memcpy(m_LODGridStart, rhs.m_LODGridStart, sizeof(m_LODGridStart));
    memcpy(m_LODGridCount, rhs.m_LODGridCount, sizeof(m_LODGridCount));
    memcpy(m_LODSkirtStart, rhs.m_LODSkirtStart, sizeof(m_LODSkirtStart));
    memcpy(m_LODSkirtCount, rhs.m_LODSkirtCount, sizeof(m_LODSkirtCount));
}

float CVIBuffer_Terrain::Get_Sample_Height(int iSampleX, int iSampleZ) const
{
    iSampleX = max(0, min(MAP - 1, iSampleX));
    iSampleZ = max(0, min(MAP - 1, iSampleZ));
    return (*m_pHeightMap)[iSampleZ * MAP + iSampleX];
}

float CVIBuffer_Terrain::Get_Terrain_Heights(float x, float z)
{
    if (nullptr == m_pHeightMap)
        return 0.f;

    if (x < -HALF || z < -HALF || x >= HALF || z >= HALF)
        return 0.f;

    int LX = int(x + HALF);
    int DZ = int(z + HALF);

    /* 예전 코드는 맵 끝에서 (DZ+1), (LX+1) 이 배열 밖을 읽었다. */
    LX = max(0, min(MAP - 2, LX));
    DZ = max(0, min(MAP - 2, DZ));

    _vector Positions[4];
    Positions[0] = XMVectorSet(LX - HALF, Get_Sample_Height(LX, DZ + 1), DZ + 1 - HALF, 1.f);
    Positions[1] = XMVectorSet(LX + 1 - HALF, Get_Sample_Height(LX + 1, DZ + 1), DZ + 1 - HALF, 1.f);
    Positions[2] = XMVectorSet(LX + 1 - HALF, Get_Sample_Height(LX + 1, DZ), DZ - HALF, 1.f);
    Positions[3] = XMVectorSet(LX - HALF, Get_Sample_Height(LX, DZ), DZ - HALF, 1.f);

    float DeltaX = x - float(LX - HALF);
    float DeltaZ = z - float(DZ - HALF);

    _vector PlaneNormal;

    if (DeltaX + DeltaZ <= 1)
        PlaneNormal = XMPlaneFromPoints(Positions[0], Positions[2], Positions[3]);
    else
        PlaneNormal = XMPlaneFromPoints(Positions[0], Positions[1], Positions[2]);

    return -(XMVectorGetX(PlaneNormal) * x + XMVectorGetZ(PlaneNormal) * z + XMVectorGetW(PlaneNormal)) / XMVectorGetY(PlaneNormal);
}

HRESULT CVIBuffer_Terrain::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CVIBuffer_Terrain::Initialize(void* pArg)
{
    /* 높이 배열도 청크 정보도 복사생성자에서 이미 받아왔다. 여기서 할 일이 없다. */
    return S_OK;
}

#pragma region 높이맵 읽기

bool CVIBuffer_Terrain::Load_HeightMap(const char* pHeightFilePath)
{
    m_pHeightMap = std::make_shared<std::vector<float>>();
    m_pHeightMap->resize(size_t(MAP) * MAP);

    std::string strPath = pHeightFilePath ? pHeightFilePath : "";

    /* 1) 높이만 들어있는 압축본(.hgt)이 있으면 그걸 읽는다. 67MB, 한 번에 읽어서 순식간이다. */
    std::string strCache = strPath;
    size_t iSlash = strCache.find_last_of("/\\");
    strCache = (iSlash == std::string::npos) ? std::string("Terrain4096.hgt")
        : strCache.substr(0, iSlash + 1) + "Terrain4096.hgt";

    {
        std::ifstream fin(strCache, std::ios::binary);
        if (fin.is_open())
        {
            _uint iSize = 0;
            fin.read(reinterpret_cast<char*>(&iSize), sizeof(_uint));
            if (iSize == MAP)
            {
                fin.read(reinterpret_cast<char*>(m_pHeightMap->data()), std::streamsize(sizeof(float) * m_pHeightMap->size()));
                if (fin.gcount() == std::streamsize(sizeof(float) * m_pHeightMap->size()))
                    return true;
            }
        }
    }

    /* 2) 없으면 원본(정점당 x,y,z float3)에서 y 만 뽑아내고, 다음부터 빠르도록 .hgt 를 남긴다.
          예전 Read_Map 은 float 하나씩 5000만번 read() 를 불러서 몇 초씩 걸렸다. 줄 단위로 읽는다. */
    std::ifstream fin(strPath, std::ios::binary);
    if (!fin.is_open())
    {
        MessageBoxA(nullptr, ("Failed to open terrain height file:\n" + strPath).c_str(), "Error", MB_OK);
        m_pHeightMap.reset();
        return false;
    }

    std::vector<float> Row(size_t(MAP) * 3);
    for (int z = 0; z < MAP; ++z)
    {
        fin.read(reinterpret_cast<char*>(Row.data()), std::streamsize(sizeof(float) * Row.size()));
        if (fin.gcount() != std::streamsize(sizeof(float) * Row.size()))
        {
            MessageBoxA(nullptr, ("Terrain height file is truncated:\n" + strPath).c_str(), "Error", MB_OK);
            m_pHeightMap.reset();
            return false;
        }

        float* pDest = m_pHeightMap->data() + size_t(z) * MAP;
        for (int x = 0; x < MAP; ++x)
            pDest[x] = Row[size_t(x) * 3 + 1];
    }
    fin.close();

    std::ofstream fout(strCache, std::ios::binary);
    if (fout.is_open())
    {
        _uint iSize = MAP;
        fout.write(reinterpret_cast<const char*>(&iSize), sizeof(_uint));
        fout.write(reinterpret_cast<const char*>(m_pHeightMap->data()), std::streamsize(sizeof(float) * m_pHeightMap->size()));
    }

    return true;
}

#pragma endregion

#pragma region 버퍼 만들기

void CVIBuffer_Terrain::Build_Vertices(std::vector<VTXMESH>& Vertices)
{
    m_ChunksPerSide = CHUNKS_PER_SIDE;
    m_VerticesPerChunk = CHUNK_VERTS;

    Vertices.resize(size_t(CHUNKS_PER_SIDE) * CHUNKS_PER_SIDE * CHUNK_VERTS);
    m_Chunks.resize(size_t(CHUNKS_PER_SIDE) * CHUNKS_PER_SIDE);

    for (int cz = 0; cz < CHUNKS_PER_SIDE; ++cz)
    {
        for (int cx = 0; cx < CHUNKS_PER_SIDE; ++cx)
        {
            const int   iChunk = cz * CHUNKS_PER_SIDE + cx;
            const int   iBaseSampleX = cx * CHUNK_WORLD;
            const int   iBaseSampleZ = cz * CHUNK_WORLD;
            VTXMESH* pChunkVtx = Vertices.data() + size_t(iChunk) * CHUNK_VERTS;

            float fMinY = FLT_MAX, fMaxY = -FLT_MAX;

            for (int lz = 0; lz < VERTS_PER_SIDE; ++lz)
            {
                for (int lx = 0; lx < VERTS_PER_SIDE; ++lx)
                {
                    const int iSampleX = iBaseSampleX + lx * STEP;
                    const int iSampleZ = iBaseSampleZ + lz * STEP;

                    const float fHeight = Get_Sample_Height(iSampleX, iSampleZ);

                    VTXMESH& Vtx = pChunkVtx[Grid_Local(lx, lz)];

                    Vtx.vPosition = XMFLOAT3(iSampleX - HALF, fHeight, iSampleZ - HALF);

                    /* 노멀/탄젠트는 높이 기울기에서 바로 구한다.
                       (예전엔 삼각형 법선을 누적했는데, 결과는 같고 이쪽이 훨씬 싸다) */
                    const float fdX = Get_Sample_Height(iSampleX - STEP, iSampleZ) - Get_Sample_Height(iSampleX + STEP, iSampleZ);
                    const float fdZ = Get_Sample_Height(iSampleX, iSampleZ - STEP) - Get_Sample_Height(iSampleX, iSampleZ + STEP);

                    XMStoreFloat3(&Vtx.vNormal, XMVector3Normalize(XMVectorSet(fdX, 2.f * STEP, fdZ, 0.f)));
                    XMStoreFloat3(&Vtx.vTangent, XMVector3Normalize(XMVectorSet(2.f * STEP, -fdX, 0.f, 0.f)));

                    /* 예전 UV 는 sx/8192 + 0.25 라서 8192짜리 지형 텍스처의 '가운데 절반'만 썼다.
                       (바깥쪽 4분의 3은 한 번도 샘플되지 않는 낭비였다)
                       그래서 텍스처를 그 가운데 4096x4096 만 잘라냈고, UV 도 0~1 로 맞춘다.
                       화면에 찍히는 텍셀 밀도는 예전과 똑같다. */
                    Vtx.vTexcoord = XMFLOAT2(iSampleX / float(MAP), iSampleZ / float(MAP));

                    fMinY = min(fMinY, fHeight);
                    fMaxY = max(fMaxY, fHeight);
                }
            }

            /* 치마 깊이는 청크마다 실측해서 정한다.
               LOD 가 한 단계 거칠어지면 테두리의 중간 정점이 직선으로 펴지면서 딱 그만큼 틈이 벌어진다.
               그 최대 편차만큼만 내리면 틈은 막히고, 평평한 청크에서는 치마가 거의 0 이 되어
               경계에 벽처럼 보이던 것이 사라진다. (예전엔 전 청크 일괄 50칸이었는데
               이 맵은 전체 기복이 51칸뿐이라 그게 그대로 격자 선으로 보였다) */
            float fSkirtDepth = 0.f;
            for (int iEdge = 0; iEdge < 4; ++iEdge)
            {
                for (int iStride = 2; iStride <= (1 << (TERRAIN_LOD_COUNT - 1)); iStride <<= 1)
                {
                    for (int i = 0; i + iStride <= CELLS; i += iStride)
                    {
                        const float fLeft = pChunkVtx[Skirt_Source(iEdge, i)].vPosition.y;
                        const float fRight = pChunkVtx[Skirt_Source(iEdge, i + iStride)].vPosition.y;

                        for (int k = 1; k < iStride; ++k)
                        {
                            const float fCoarse = fLeft + (fRight - fLeft) * (float(k) / iStride);
                            const float fFine = pChunkVtx[Skirt_Source(iEdge, i + k)].vPosition.y;

                            /* 이 청크가 고운 쪽이든 거친 쪽이든 틈은 생기므로 절대값으로 잡는다. */
                            fSkirtDepth = max(fSkirtDepth, fabsf(fFine - fCoarse));
                        }
                    }
                }
            }
            fSkirtDepth = min(fSkirtDepth * 1.1f + 0.05f, float(TERRAIN_SKIRT_DEPTH));

            /* 치마 정점 : 테두리 정점을 그대로 베끼고 그만큼 아래로 내린다. */
            for (int iEdge = 0; iEdge < 4; ++iEdge)
            {
                for (int i = 0; i < VERTS_PER_SIDE; ++i)
                {
                    VTXMESH& Skirt = pChunkVtx[Skirt_Local(iEdge, i)];
                    Skirt = pChunkVtx[Skirt_Source(iEdge, i)];
                    Skirt.vPosition.y -= fSkirtDepth;
                }
            }

            TERRAINCHUNK& Chunk = m_Chunks[iChunk];
            Chunk.BaseVertex = _uint(size_t(iChunk) * CHUNK_VERTS);
            Chunk.vCenter = XMFLOAT3(iBaseSampleX + CHUNK_WORLD * 0.5f - HALF,
                (fMinY + fMaxY) * 0.5f,
                iBaseSampleZ + CHUNK_WORLD * 0.5f - HALF);
            Chunk.vExtents = XMFLOAT3(CHUNK_WORLD * 0.5f,
                (fMaxY - fMinY) * 0.5f + TERRAIN_SKIRT_DEPTH,
                CHUNK_WORLD * 0.5f);
        }
    }
}

void CVIBuffer_Terrain::Build_Indices(std::vector<_ushort>& Indices)
{
    Indices.clear();
    Indices.reserve(CELLS * CELLS * 6 * 2);

    for (_uint iLOD = 0; iLOD < TERRAIN_LOD_COUNT; ++iLOD)
    {
        const int iStride = 1 << iLOD;

        m_LODGridStart[iLOD] = _uint(Indices.size());

        /* 격자 - 감기 순서는 예전 Make_Buffer 와 똑같이 맞춘다(안 그러면 뒷면이 잘린다) */
        for (int z = 0; z + iStride <= CELLS; z += iStride)
        {
            for (int x = 0; x + iStride <= CELLS; x += iStride)
            {
                const _ushort i0 = _ushort(Grid_Local(x, z + iStride));
                const _ushort i1 = _ushort(Grid_Local(x + iStride, z + iStride));
                const _ushort i2 = _ushort(Grid_Local(x + iStride, z));
                const _ushort i3 = _ushort(Grid_Local(x, z));

                Indices.push_back(i0); Indices.push_back(i1); Indices.push_back(i2);
                Indices.push_back(i0); Indices.push_back(i2); Indices.push_back(i3);
            }
        }

        m_LODGridCount[iLOD] = _uint(Indices.size()) - m_LODGridStart[iLOD];

        /* 치마 - 변마다 따로 잡아둔다. 이웃과 LOD 가 다른 변에만 그릴 것이다.
           어느 쪽에서 보든 틈이 막히도록 양면으로 깐다(개수가 적어서 부담이 없다). */
        for (int iEdge = 0; iEdge < 4; ++iEdge)
        {
            m_LODSkirtStart[iLOD][iEdge] = _uint(Indices.size());

            for (int i = 0; i + iStride <= CELLS; i += iStride)
            {
                const _ushort iTop0 = _ushort(Skirt_Source(iEdge, i));
                const _ushort iTop1 = _ushort(Skirt_Source(iEdge, i + iStride));
                const _ushort iBot0 = _ushort(Skirt_Local(iEdge, i));
                const _ushort iBot1 = _ushort(Skirt_Local(iEdge, i + iStride));

                Indices.push_back(iTop0); Indices.push_back(iTop1); Indices.push_back(iBot1);
                Indices.push_back(iTop0); Indices.push_back(iBot1); Indices.push_back(iBot0);

                Indices.push_back(iTop0); Indices.push_back(iBot1); Indices.push_back(iTop1);
                Indices.push_back(iTop0); Indices.push_back(iBot0); Indices.push_back(iBot1);
            }

            m_LODSkirtCount[iLOD][iEdge] = _uint(Indices.size()) - m_LODSkirtStart[iLOD][iEdge];
        }
    }
}

void CVIBuffer_Terrain::Make_Buffer(const char* pHeightFilePath)
{
    if (!Load_HeightMap(pHeightFilePath))
        return;

    std::vector<VTXMESH> Vertices;
    Build_Vertices(Vertices);

    std::vector<_ushort> Indices;
    Build_Indices(Indices);

    m_VertexByteStride = sizeof(VTXMESH);
    m_VertexNum = _uint(Vertices.size());
    m_VertexBufferByteSize = m_VertexNum * m_VertexByteStride;

    m_IndexFormat = DXGI_FORMAT_R16_UINT;
    m_IndexNum = _uint(Indices.size());
    m_IndexBufferByteSize = m_IndexNum * sizeof(_ushort);

    m_PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    /* Create_Buffer 는 안에서 Execute/Flush 까지 하고 업로드 버퍼도 풀어준다. */
    __super::Create_Buffer(&m_VertexBufferGPU, &m_VertexBufferUploader, Vertices.data(), m_VertexBufferByteSize);
    __super::Create_Buffer(&m_IndexBufferGPU, &m_IndexBufferUploader, Indices.data(), m_IndexBufferByteSize);
}

#pragma endregion

#pragma region 컬링 / LOD

void CVIBuffer_Terrain::Extract_FrustumPlanes(_fmatrix ViewProj)
{
    _float4x4 M;
    XMStoreFloat4x4(&M, ViewProj);

    /* 행벡터(v * M) 규약. 좌 = col3+col0, 우 = col3-col0, ... , 근 = col2, 원 = col3-col2 */
    const float p[6][4] =
    {
        { M._14 + M._11, M._24 + M._21, M._34 + M._31, M._44 + M._41 },
        { M._14 - M._11, M._24 - M._21, M._34 - M._31, M._44 - M._41 },
        { M._14 + M._12, M._24 + M._22, M._34 + M._32, M._44 + M._42 },
        { M._14 - M._12, M._24 - M._22, M._34 - M._32, M._44 - M._42 },
        { M._13,         M._23,         M._33,         M._43         },
        { M._14 - M._13, M._24 - M._23, M._34 - M._33, M._44 - M._43 },
    };

    for (int i = 0; i < 6; ++i)
    {
        const float fLength = sqrtf(p[i][0] * p[i][0] + p[i][1] * p[i][1] + p[i][2] * p[i][2]);
        if (fLength < 1e-6f)
        {
            m_isCullingValid = false;
            return;
        }
        m_FrustumPlanes[i] = XMFLOAT4(p[i][0] / fLength, p[i][1] / fLength, p[i][2] / fLength, p[i][3] / fLength);
    }

    m_isCullingValid = true;
}

bool CVIBuffer_Terrain::Is_Chunk_Visible(const TERRAINCHUNK& Chunk) const
{
    if (!m_isCullingValid)
        return true;

    for (int i = 0; i < 6; ++i)
    {
        const XMFLOAT4& P = m_FrustumPlanes[i];

        const float fDist = P.x * Chunk.vCenter.x + P.y * Chunk.vCenter.y + P.z * Chunk.vCenter.z + P.w;
        const float fRadius = fabsf(P.x) * Chunk.vExtents.x + fabsf(P.y) * Chunk.vExtents.y + fabsf(P.z) * Chunk.vExtents.z;

        if (fDist + fRadius < 0.f)
            return false;
    }

    return true;
}

_uint CVIBuffer_Terrain::Pick_LOD(const TERRAINCHUNK& Chunk, const XMFLOAT3& vEye) const
{
    const float fdX = Chunk.vCenter.x - vEye.x;
    const float fdY = Chunk.vCenter.y - vEye.y;
    const float fdZ = Chunk.vCenter.z - vEye.z;

    const float fDistance = sqrtf(fdX * fdX + fdY * fdY + fdZ * fdZ);

    /* 청크 하나 크기의 2배 안쪽이면 LOD0, 그 다음 2배마다 한 단계씩 거칠어진다. */
    float fRatio = fDistance / (CHUNK_WORLD * 2.f);

    _uint iLOD = 0;
    while (fRatio >= 1.f && iLOD < TERRAIN_LOD_COUNT - 1)
    {
        fRatio *= 0.5f;
        ++iLOD;
    }

    return iLOD;
}

#pragma endregion

HRESULT CVIBuffer_Terrain::Render(int instanceNum)
{
    if (m_Chunks.empty())
        return S_OK;

    CGameInstance* pGameInstance = CGameInstance::Get_Instance();

    /* 그림자 패스에서는 빛의 절두체로 자른다. 빛은 탱크 주변 600칸만 비추므로 청크 몇 개면 끝난다.
       LOD 는 두 패스 모두 '카메라' 기준으로 골라야 그림자와 지형이 어긋나지 않는다. */
    const bool isShadowPass = pGameInstance->Is_ShadowPass();

    if (pGameInstance->Is_CameraValid())
        Extract_FrustumPlanes(isShadowPass ? pGameInstance->Get_ShadowViewProj() : pGameInstance->Get_CameraViewProj());
    else
        m_isCullingValid = false;

    const XMFLOAT3 vEye = pGameInstance->Get_CameraEye();

    m_CommandList->IASetVertexBuffers(0, 1, &VertexBufferView());
    m_CommandList->IASetIndexBuffer(&IndexBufferView());
    m_CommandList->IASetPrimitiveTopology(m_PrimitiveType);

    m_DrawnChunks = 0;
    m_DrawnTriangles = 0;

    /* LOD 는 카메라만 보고 정해지므로 이웃 청크의 LOD 도 그냥 계산해두면 된다.
       (그림자 패스에서도 같은 값이 나와야 지형과 그림자가 어긋나지 않는다) */
    m_ChunkLODs.resize(m_Chunks.size());
    for (size_t i = 0; i < m_Chunks.size(); ++i)
        m_ChunkLODs[i] = Pick_LOD(m_Chunks[i], vEye);

    const int iChunksPerSide = int(m_ChunksPerSide);

    for (int cz = 0; cz < iChunksPerSide; ++cz)
    {
        for (int cx = 0; cx < iChunksPerSide; ++cx)
        {
            const int iChunk = cz * iChunksPerSide + cx;
            const TERRAINCHUNK& Chunk = m_Chunks[iChunk];

            if (!Is_Chunk_Visible(Chunk))
                continue;

            const _uint iLOD = m_ChunkLODs[iChunk];

            m_CommandList->DrawIndexedInstanced(m_LODGridCount[iLOD], instanceNum,
                m_LODGridStart[iLOD], INT(Chunk.BaseVertex), 0);

            ++m_DrawnChunks;
            m_DrawnTriangles += m_LODGridCount[iLOD] / 3;

            /* 이웃과 LOD 가 다른 변에만 치마를 세운다. 거기가 실제로 틈이 벌어지는 자리다.
               같은 LOD 끼리는 정점이 정확히 겹치므로 치마가 필요 없고,
               괜히 세우면 경계마다 벽이 보인다. */
            const int iNeighbours[4][2] = { { cx, cz - 1 }, { cx, cz + 1 }, { cx - 1, cz }, { cx + 1, cz } };

            for (int iEdge = 0; iEdge < 4; ++iEdge)
            {
                const int nx = iNeighbours[iEdge][0];
                const int nz = iNeighbours[iEdge][1];

                if (nx < 0 || nz < 0 || nx >= iChunksPerSide || nz >= iChunksPerSide)
                    continue;

                if (iLOD == m_ChunkLODs[nz * iChunksPerSide + nx])
                    continue;

                m_CommandList->DrawIndexedInstanced(m_LODSkirtCount[iLOD][iEdge], instanceNum,
                    m_LODSkirtStart[iLOD][iEdge], INT(Chunk.BaseVertex), 0);

                m_DrawnTriangles += m_LODSkirtCount[iLOD][iEdge] / 3;
            }
        }
    }

    return S_OK;
}

CVIBuffer_Terrain* CVIBuffer_Terrain::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, const char* pHeightFilePath)
{
    CVIBuffer_Terrain* pInstance = new CVIBuffer_Terrain(pDevice, pCommandList);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        return nullptr;
    }
    pInstance->Make_Buffer(pHeightFilePath);
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
    m_Chunks.clear();
    m_pHeightMap.reset();

    CVIBuffer::Free();
}
