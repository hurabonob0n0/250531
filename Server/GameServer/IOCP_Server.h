#pragma once
#include "Session.h"
#include <vector>
#include <thread>

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
