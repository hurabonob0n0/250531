#include "pch.h"
#include "IOCP_Server.h"
#include "Session_Manager.h"
#include "Room_Manager.h"
#include "Terrain_Manager.h"

int main()
{
    std::cout << "=== JOL GameServer ===" << std::endl;

    // 어떤 설정으로 빌드된 서버인지 먼저 출력한다.
    PrintServerConfig();

    // 지형 높이맵
    // 클라가 렌더링에 쓰는 Terrain4096.hgt 를 그대로 읽는다

    static const char* TERRAIN_CANDIDATES[] =
    {
        "Terrain4096.hgt",
        "../../Client/MainApp/bin/Models/Terrain/Terrain4096.hgt",
        "../../../Client/MainApp/bin/Models/Terrain/Terrain4096.hgt",
        "Terrain4096Map.bin",       // 정점 x,y,z 원본
    };

    std::cout << "맵 데이터 읽는 중..." << std::endl;

    bool isTerrainLoaded = false;
    for (const char* pPath : TERRAIN_CANDIDATES)
    {
        if (Terrain_Manager::GetInstance().Read_Map(pPath, 4096, 4096, 1.0f))
        {
            std::cout << "맵 읽기 완료 : " << pPath << std::endl;
            isTerrainLoaded = true;
            break;
        }
    }

    if (!isTerrainLoaded)
        std::cout << "[경고] 지형 파일을 찾지 못했습니다 - 지형 높이는 0으로 처리됩니다." << std::endl;

    // 워커 스레드가 뜨기 전에 미리 만들어 둔다(경합 없이 안전하게 초기화).
    Room_Manager::Get_Instance();
    CSession_Manager::Get_Instance();

    CIOCP_Server server;
    if (!server.Start(SERVER_PORT))
    {
        std::cout << "서버 시작 실패" << std::endl;
        Room_Manager::Destroy_Instance();
        CSession_Manager::Destroy_Instance();
        return -1;
    }

    server.Run();   // 워커 스레드 join

    Room_Manager::Destroy_Instance();
    CSession_Manager::Destroy_Instance();
    return 0;
}
