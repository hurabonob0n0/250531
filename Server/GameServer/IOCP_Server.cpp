#include "pch.h"
#include "IOCP_Server.h"
#include "Session_Manager.h"
#include "Room_Manager.h"
#include "Protocol.h"

CIOCP_Server::CIOCP_Server() {}

CIOCP_Server::~CIOCP_Server()
{
    if (m_listenSocket != INVALID_SOCKET)
        closesocket(m_listenSocket);
    if (m_hIOCP != INVALID_HANDLE_VALUE)
        CloseHandle(m_hIOCP);
    WSACleanup();
}

bool CIOCP_Server::Start(uint16_t nPort)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "[CIOCP_Server] WSAStartup 실패" << std::endl;
        return false;
    }

    if (!InitIOCP())        return false;
    if (!InitSocket(nPort)) return false;

    // AcceptEx 는 확장 함수라 MS 권장대로 함수 주소를 받아 쓴다.
    GUID  guidAcceptEx = WSAID_ACCEPTEX;
    DWORD dwBytes      = 0;
    WSAIoctl(m_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
             &guidAcceptEx, sizeof(guidAcceptEx),
             &m_fnAcceptEx, sizeof(m_fnAcceptEx),
             &dwBytes, nullptr, nullptr);

    if (m_fnAcceptEx == nullptr)
    {
        std::cout << "[CIOCP_Server] AcceptEx 포인터 획득 실패" << std::endl;
        return false;
    }

    StartAccept();

    m_gameThread  = std::thread(&CIOCP_Server::GameLoopThread, this);
    m_debugThread = std::thread(&CIOCP_Server::DebugConsoleThread, this);
    m_gameThread.detach();
    m_debugThread.detach();

    const int32_t nThreadCount =
        static_cast<int32_t>(std::thread::hardware_concurrency());

    for (int32_t i = 0; i < nThreadCount; ++i)
        m_workerThreads.emplace_back(&CIOCP_Server::WorkerThread, this);

    std::cout << "[CIOCP_Server] 포트 " << nPort << " 대기 중 (워커 "
              << nThreadCount << "개)" << std::endl;
    return true;
}

void CIOCP_Server::Run()
{
    for (auto& t : m_workerThreads)
        t.join();
}

bool CIOCP_Server::InitIOCP()
{
    m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (m_hIOCP == nullptr)
    {
        std::cout << "[CIOCP_Server] IOCP 생성 실패" << std::endl;
        return false;
    }
    return true;
}

bool CIOCP_Server::InitSocket(uint16_t nPort)
{
    m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
                               nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (m_listenSocket == INVALID_SOCKET)
    {
        std::cout << "[CIOCP_Server] 소켓 생성 실패" << std::endl;
        return false;
    }

    // 서버를 재시작할 때 OS 가 포트를 붙잡고 있어 bind 가 거부되는 것을 막는다.
    BOOL bReuseAddr = TRUE;
    setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&bReuseAddr), sizeof(bReuseAddr));

    // 리슨 소켓을 IOCP 에 등록. key=0 으로 등록하고
    // Accept 완료는 key 가 아니라 CIOEvent 의 IOType 으로 구분한다.
    CreateIoCompletionPort(
        reinterpret_cast<HANDLE>(m_listenSocket), m_hIOCP, 0, 0);

    SOCKADDR_IN addr = {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(nPort);
    addr.sin_addr.s_addr = INADDR_ANY;

    // pch.h 의 using namespace std 때문에 그냥 bind 라고 쓰면 std::bind 로 잡힌다.
    if (::bind(m_listenSocket,
               reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) == SOCKET_ERROR)
    {
        std::cout << "[CIOCP_Server] bind 실패: " << WSAGetLastError() << std::endl;
        return false;
    }

    if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cout << "[CIOCP_Server] listen 실패: " << WSAGetLastError() << std::endl;
        return false;
    }

    return true;
}

// ----------------------------------------------------------------
//  세션 슬롯 ACCEPT_POOL_SIZE 개를 미리 준비해 AcceptEx 를 걸어둔다.
//  접속이 들어와도 그 자리에서 소켓을 만들지 않으므로 지연이 없다.
// ----------------------------------------------------------------
void CIOCP_Server::StartAccept()
{
    for (int32_t i = 0; i < ACCEPT_POOL_SIZE; ++i)
    {
        int32_t nID = CSession_Manager::Get_Instance()->Assign();
        if (nID == -1) break;

        SessionRef pSession = CSession_Manager::Get_Instance()->Get_Session(nID);

        SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
                                        nullptr, 0, WSA_FLAG_OVERLAPPED);
        pSession->SetSocket(clientSocket);

        // IOCP 등록은 소켓 생성 직후 한 번만.
        CreateIoCompletionPort(
            reinterpret_cast<HANDLE>(clientSocket),
            m_hIOCP,
            static_cast<ULONG_PTR>(nID), 0);

        ReRegisterAccept(pSession);
    }
}

void CIOCP_Server::ReRegisterAccept(SessionRef pSession)
{
    pSession->GetAcceptEvent()->Reset();
    pSession->Initialize();

    DWORD dwBytesReceived = 0;
    int   nAddrSize       = sizeof(SOCKADDR_IN);

    BOOL bResult = m_fnAcceptEx(
        m_listenSocket,
        pSession->GetSocket(),
        pSession->GetAcceptBuf(),
        0,                      // 접속만 받고 데이터는 기다리지 않는다
        nAddrSize + 16,
        nAddrSize + 16,
        &dwBytesReceived,
        &pSession->GetAcceptEvent()->m_overlapped);

    if (bResult == FALSE)
    {
        int nErr = WSAGetLastError();
        if (nErr != WSA_IO_PENDING)
            std::cout << "[ReRegisterAccept] AcceptEx 실패: " << nErr
                      << " Socket=" << pSession->GetSocket() << std::endl;
    }
}

// ----------------------------------------------------------------
//  워커 스레드 - GQCS 로 받은 CIOEvent 의 IOType 으로 분기
// ----------------------------------------------------------------
void CIOCP_Server::WorkerThread()
{
    while (true)
    {
        DWORD       dwNumOfBytes = 0;
        ULONG_PTR   ulKey        = 0;
        OVERLAPPED* pOver        = nullptr;

        BOOL bResult = GetQueuedCompletionStatus(
            m_hIOCP, &dwNumOfBytes, &ulKey, &pOver, INFINITE);

        CIOEvent* pIOEvent = reinterpret_cast<CIOEvent*>(pOver);
        if (pIOEvent == nullptr) continue;

        CSession* pRawSession = pIOEvent->m_owner;
        if (pRawSession == nullptr) continue;

        // 슬롯에서 shared_ptr 을 다시 꺼낸다.
        // 처리 도중 다른 스레드가 세션을 반납해도 여기서 잡은 참조 덕에 살아 있다.
        SessionRef pSession =
            CSession_Manager::Get_Instance()->Get_Session(pRawSession->GetID());

        if (pIOEvent->m_type == IOType::Accept)
        {
            // Accept 는 bResult 가 TRUE 면 성공이다.
            // 데이터 없이 연결만 받았으므로 dwNumOfBytes == 0 이 정상.
            if (bResult == TRUE)
            {
                if (pSession) ProcessAccept(pSession);
            }
            else
            {
                if (pSession && pSession->GetSocket() != INVALID_SOCKET)
                    ReRegisterAccept(pSession);
            }
            continue;
        }

        // Recv 가 0바이트면 상대가 정상 종료한 것이다.
        if (bResult == FALSE || dwNumOfBytes == 0)
        {
            if (pSession) pSession->Disconnect();
            continue;
        }

        switch (pIOEvent->m_type)
        {
        case IOType::Recv:
            if (pSession) ProcessRecv(pSession, static_cast<int32_t>(dwNumOfBytes));
            break;
        case IOType::Send:
            if (pSession) ProcessSend(pSession);
            break;
        default:
            break;
        }
    }
}

void CIOCP_Server::ProcessAccept(SessionRef pSession)
{
    // AcceptEx 로 받은 소켓은 이 후처리를 해야 리슨 소켓의 속성을 물려받는다.
    setsockopt(pSession->GetSocket(),
               SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
               reinterpret_cast<const char*>(&m_listenSocket),
               sizeof(m_listenSocket));

    // 게임 패킷은 전부 수십~수백 바이트짜리라 Nagle 이 켜져 있으면 밀린다.
    BOOL bNoDelay = TRUE;
    setsockopt(pSession->GetSocket(), IPPROTO_TCP, TCP_NODELAY,
               reinterpret_cast<const char*>(&bNoDelay), sizeof(bNoDelay));

    pSession->SetConnected(true);
    CSession_Manager::Get_Instance()->OnConnected();
    pSession->RegisterRecv();

    // 방금 쓴 슬롯을 대신할 대기 세션을 보충한다.
    int32_t nNewID = CSession_Manager::Get_Instance()->Assign();
    if (nNewID != -1)
    {
        SessionRef pNewSession =
            CSession_Manager::Get_Instance()->Get_Session(nNewID);

        SOCKET newSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
                                     nullptr, 0, WSA_FLAG_OVERLAPPED);
        pNewSession->SetSocket(newSocket);

        CreateIoCompletionPort(
            reinterpret_cast<HANDLE>(newSocket),
            m_hIOCP,
            static_cast<ULONG_PTR>(nNewID), 0);

        ReRegisterAccept(pNewSession);
    }
}

void CIOCP_Server::ProcessRecv(SessionRef pSession, int32_t nNumOfBytes)
{
    pSession->OnRecvComplete(nNumOfBytes);
}

void CIOCP_Server::ProcessSend(SessionRef pSession)
{
    pSession->OnSendComplete();
}

// ----------------------------------------------------------------
//  게임 루프 스레드
//  예전에는 main 이 직접 돌렸다. 스레드로 떼어내야 main 이
//  서버 수명(워커 join)만 책임지는 구조가 된다.
// ----------------------------------------------------------------
void CIOCP_Server::GameLoopThread()
{
    using clock = std::chrono::high_resolution_clock;

    const float fTargetFrameTime = 1.0f / static_cast<float>(GAME_TICK_FPS);
    auto        previousTime     = clock::now();

    while (true)
    {
        auto  currentTime = clock::now();
        float fDeltaTime  =
            std::chrono::duration<float>(currentTime - previousTime).count();
        previousTime = currentTime;

        Room_Manager::Get_Instance()->Update(fDeltaTime);
        Room_Manager::Get_Instance()->Late_Update();

        // 프레임에 남은 시간만큼만 잔다.
        // (예전 코드는 목표시간에서 "직전 프레임 소요"를 뺐기 때문에
        //  실제 이번 프레임에 쓴 시간과 무관한 값으로 자고 있었다)
        float fElapsed = std::chrono::duration<float>(
            clock::now() - currentTime).count();
        float fSleep = fTargetFrameTime - fElapsed;

        if (fSleep > 0.f)
            std::this_thread::sleep_for(std::chrono::duration<float>(fSleep));
    }
}

// ----------------------------------------------------------------
//  콘솔 전체를 공백으로 덮고 커서를 좌상단으로 되돌린다.
//  기동 로그를 한 번에 치우는 용도.
// ----------------------------------------------------------------
static void ClearConsoleAll(HANDLE hConsole)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;

    const DWORD cells = static_cast<DWORD>(csbi.dwSize.X) * csbi.dwSize.Y;
    const COORD home  = { 0, 0 };
    DWORD       written = 0;

    FillConsoleOutputCharacterA(hConsole, ' ', cells, home, &written);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cells, home, &written);
    SetConsoleCursorPosition(hConsole, home);
}

// ----------------------------------------------------------------
//  디버그 콘솔 스레드
//
//  ★ 예전에는 Room_Manager::Update 가 매 프레임 system("cls") 를 부르고
//    방 목록을 cout 으로 찍었다. 초당 60번 프로세스를 새로 띄우는 셈이라
//    게임 루프가 콘솔에 묶여 있었다.
//    여기서는 프레임 전체를 문자열로 만들어 DEBUG_PRINT_MS 마다 한 번만
//    출력하고, 줄마다 콘솔 폭까지 공백을 채워 잔상을 지운다.
// ----------------------------------------------------------------
void CIOCP_Server::DebugConsoleThread()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 기동 로그를 한 번 치우고 대시보드로 전환한다.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ClearConsoleAll(hConsole);

    while (true)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        int nWidth = 80;
        if (GetConsoleScreenBufferInfo(hConsole, &csbi))
        {
            nWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            if (nWidth < 50) nWidth = 50;
        }

        const int32_t nConnected = CSession_Manager::Get_Instance()->GetConnectedCount();
        const int32_t nAssigned  = CSession_Manager::Get_Instance()->GetCount();

        std::ostringstream oss;

        auto line = [&](const std::string& text)
        {
            std::string s = text;
            if (static_cast<int>(s.size()) < nWidth)
                s.append(static_cast<size_t>(nWidth) - s.size(), ' ');
            oss << s << "\n";
        };

        line("=== JOL GameServer [" + GetServerConfigTag() + "] ===");
        {
            std::ostringstream l;
            l << " 접속 " << nConnected
              << "  / 대기 슬롯 " << (nAssigned - nConnected)
              << "  / 포트 " << SERVER_PORT;
            line(l.str());
        }
        line("");
        line(" ROOM   PLAYERS   STATE");

        std::vector<Room_Data> rooms = Room_Manager::Get_Instance()->Client_ShowRoom();
        for (const Room_Data& room : rooms)
        {
            std::ostringstream l;
            l << "  " << std::setw(3) << static_cast<int>(room.RoomID)
              << "     " << std::setw(2) << static_cast<int>(room.CurPlayer)
              << " / "   << std::setw(2) << static_cast<int>(room.MaxPlayer)
              << "     " << (room.isActive ? "ACTIVE" : "idle");
            line(l.str());
        }

        SetConsoleCursorPosition(hConsole, { 0, 0 });
        std::cout << oss.str() << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(DEBUG_PRINT_MS));
    }
}
