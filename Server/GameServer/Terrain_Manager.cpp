#include "pch.h"
#include "Terrain_Manager.h"
#include "fstream"

#define NUMVERTERTICES 4096

bool Terrain_Manager::Read_Map(const std::string& filePath, int width, int height, float cellSpacing)
{
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open())
        return false;

    m_width = width;
    m_height = height;
    m_cellSpacing = cellSpacing;

    m_heightMap.resize(size_t(width) * height);

    // ------------------------------------------------------------
    //  형식 1 : Terrain4096.hgt  - 클라가 쓰는 것과 같은 파일
    //
    //  [uint32 MAP][float MAP*MAP]  이고 순서는 [z * MAP + x] 로 여기와 같다.
    //  클라 렌더링(CVIBuffer_Terrain::Load_HeightMap)과 완전히 같은 데이터를
    //  읽으므로 서버와 클라의 지형이 어긋날 수 없다.
    //
    //  탱크가 실제로 굴러다니는 PhysX 하이트필드(HeightD.png)와도
    //  높이 17.442~68.544 vs 17.440~68.540 으로 일치한다(최대차 0.008).
    //  그 0.008 은 PhysX 가 샘플을 PxI16 으로 담으며 소수부를 버리는 양이다.
    // ------------------------------------------------------------
    uint32_t magic = 0;
    inFile.read(reinterpret_cast<char*>(&magic), sizeof(magic));

    if (inFile.gcount() == sizeof(magic) && magic == static_cast<uint32_t>(width))
    {
        const std::streamsize need =
            static_cast<std::streamsize>(sizeof(float) * m_heightMap.size());

        inFile.read(reinterpret_cast<char*>(m_heightMap.data()), need);

        if (inFile.gcount() == need)
            return true;

        // 잘린 파일. 0 높이로 도는 것보다 실패를 알리는 편이 낫다.
        m_heightMap.clear();
        return false;
    }

    // ------------------------------------------------------------
    //  형식 2 : Terrain4096Map.bin - 정점마다 x,y,z 세 개가 들어있는 원본
    //
    //  y 만 필요하지만 형식이 그렇게 생겼다. 예전에는 float 하나씩 read() 를
    //  5000만 번 불렀는데, 줄 단위로 읽으면 같은 결과가 훨씬 빨리 나온다.
    // ------------------------------------------------------------
    inFile.clear();
    inFile.seekg(0, std::ios::beg);

    std::vector<float> row(size_t(width) * 3);

    for (int z = 0; z < height; ++z)
    {
        const std::streamsize need =
            static_cast<std::streamsize>(sizeof(float) * row.size());

        inFile.read(reinterpret_cast<char*>(row.data()), need);

        if (inFile.gcount() != need)
        {
            m_heightMap.clear();
            return false;
        }

        for (int x = 0; x < width; ++x)
            m_heightMap[Get_Index(x, z)] = row[size_t(x) * 3 + 1];
    }

    return true;
}


//float Terrain_Manager::Get_Height(float worldX, float worldZ) const
//{
//    int x = static_cast<int>((worldX / m_cellSpacing) + (m_width / 2));
//    int z = static_cast<int>((worldZ / m_cellSpacing) + (m_height / 2));
//
//    if (x < 0 || x >= m_width || z < 0 || z >= m_height)
//        return 0.0f;
//
//    return m_heightMap[Get_Index(x, z)];
//}

void Terrain_Manager::Show_MapData()
{

    for (int z = 0; z < 4096; ++z)
    {
        for (int x = 0; x < 4096; ++x)
        {
            int idx = Get_Index(x, z);
            float height = m_heightMap[idx];
            std::cout << "X : " << x << "      Z: " << z << "      Y: " << height << endl;
        }
    }
}



// 세 점으로 평면의 방정식을 구하는 함수 (XMPlaneFromPoints 대체)
// 평면 방정식: Ax + By + Cz + D = 0
// 반환값: Vector4 { A, B, C, D }
Vector4 Terrain_Manager::PlaneFromPoints(const Vector4& p1, const Vector4& p2, const Vector4& p3)
{
    // 평면 위의 두 벡터를 생성합니다 (p1->p2, p1->p3)
    Vector4 v1 = { p2.x - p1.x, p2.y - p1.y, p2.z - p1.z, 0.f };
    Vector4 v2 = { p3.x - p1.x, p3.y - p1.y, p3.z - p1.z, 0.f };

    // 외적(Cross Product)을 통해 평면의 법선 벡터 N(A, B, C)를 구합니다.
    Vector4 normal;
    normal.x = (v1.y * v2.z) - (v1.z * v2.y); // A
    normal.y = (v1.z * v2.x) - (v1.x * v2.z); // B
    normal.z = (v1.x * v2.y) - (v1.y * v2.x); // C

    // 법선 벡터를 정규화(Normalize)합니다. (길이를 1로 만듦)
    float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length > 1e-6f) { // 0으로 나누는 것을 방지
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
    }

    // D 값을 계산합니다: D = -(N · p1) = -(A*x1 + B*y1 + C*z1)
    normal.w = -(normal.x * p1.x + normal.y * p1.y + normal.z * p1.z); // D

    return normal;
}

// 요청하신 함수를 수정한 버전입니다.
// (클래스 멤버 함수이므로, 실제 사용 시에는 클래스 정의 안에 포함해야 합니다)
float Terrain_Manager::Get_Height(float x, float z)
{
    // 높이맵을 못 읽었으면(파일 부재 등) 아래에서 빈 vector 를 인덱싱한다.
    // Read_Map 실패는 서버 기동을 막지 않으므로 여기서 걸러야 한다.
    if (m_heightMap.empty())
        return 0.f;

    // NUMVERTERTICES가 클래스 멤버 변수라고 가정합니다.
    float half = NUMVERTERTICES / 2.f;

    int LX = static_cast<int>(x + half);
    int DZ = static_cast<int>(z + half);

    // 아래에서 (LX + 1, DZ + 1) 까지 읽으므로 마지막 칸은 제외해야 한다.
    // 예전 조건(x > half)은 x = 2047.9 를 통과시켰고, 그러면 LX + 1 이 4096 이 되어
    // 배열 밖을 읽었다. 맵 가장자리에서만 나던 문제라 눈에 안 띄었다.
    if (LX < 0 || DZ < 0 || LX >= m_width - 1 || DZ >= m_height - 1)
        return 0.f;

    // _vector 배열을 Vector4 배열로 변경
    Vector4 Positions[4];

    // XMVectorSet을 구조체 초기화로 변경
    Positions[0] = { LX - half,       m_heightMap[(DZ + 1) * NUMVERTERTICES + LX],     DZ + 1 - half, 1.f };
    Positions[1] = { LX + 1 - half,   m_heightMap[(DZ + 1) * NUMVERTERTICES + LX + 1], DZ + 1 - half, 1.f };
    Positions[2] = { LX + 1 - half,   m_heightMap[DZ * NUMVERTERTICES + LX + 1],       DZ - half,     1.f };
    Positions[3] = { LX - half,       m_heightMap[DZ * NUMVERTERTICES + LX],           DZ - half,     1.f };

    float DeltaX = x - (LX - half);
    float DeltaZ = z - (DZ - half);

    // _vector를 Vector4로 변경
    Vector4 PlaneNormal;

    // 직접 구현한 PlaneFromPoints 함수를 호출
    if (DeltaX + DeltaZ <= 1.0f)
        PlaneNormal = PlaneFromPoints(Positions[0], Positions[2], Positions[3]);
    else
        PlaneNormal = PlaneFromPoints(Positions[0], Positions[1], Positions[2]);

    // XMVectorGetX/Y/Z/W를 구조체 멤버 접근으로 변경
    // 평면 방정식(Ax + By + Cz + D = 0)을 y에 대해 정리: y = -(Ax + Cz + D) / B
    float B = PlaneNormal.y;

    return -(PlaneNormal.x * x + PlaneNormal.z * z + PlaneNormal.w) / B;
}