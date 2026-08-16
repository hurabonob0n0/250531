#pragma once
#include "Session.h"
#include <vector>
#include <thread>

// ================================================================
//  CIOCP_Server
//
//  ServerCore 의 IocpCore + Listener + Service + ThreadManager 를
//  한 클래스로 합친 것이다. 예전에는 main 에서 ServerService 를 조립하고
//  워커 스레드가 service->GetIocpCore()->Dispatch() 를 무한 호출했는데,
//  여기서는 이 클래스가 리슨 소켓 / AcceptEx 풀 / 워커 / 게임 루프 /
//  디버그 콘솔을 전부 들고 있다.
//
//  스레드 구성
//    - 워커      : 하드웨어 코어 수만큼. GQCS 로 완료 통지를 받아 처리.
//    - 게임 루프 : 1개. Room_Manager 를 GAME_TICK_FPS 로 돌린다.
//    - 디버그    : 1개. 콘솔 대시보드를 DEBUG_PRINT_MS 마다 다시 그린다.
// ================================================================
class CIOCP_Server
{
public:
    CIOCP_Server();
    ~CIOCP_Server();

    bool Start(uint16_t nPort);
    void Run();

    HANDLE GetIOCPHandle() { return m_hIOCP; }

private:
    bool InitSocket(uint16_t nPort);
    bool InitIOCP();
    void StartAccept();
    void ReRegisterAccept(SessionRef pSession);

    void WorkerThread();
    void GameLoopThread();
    void DebugConsoleThread();

    void ProcessAccept(SessionRef pSession);
    void ProcessRecv(SessionRef pSession, int32_t nNumOfBytes);
    void ProcessSend(SessionRef pSession);

    LPFN_ACCEPTEX m_fnAcceptEx = nullptr;

    HANDLE m_hIOCP        = INVALID_HANDLE_VALUE;
    SOCKET m_listenSocket = INVALID_SOCKET;

    std::vector<std::thread> m_workerThreads;
    std::thread              m_gameThread;
    std::thread              m_debugThread;
};
