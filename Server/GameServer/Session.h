#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <MSWSock.h>
#include <Windows.h>

#include <cstdint>
#include <atomic>
#include <mutex>
#include <memory>
#include <queue>

#include "SendBuffer.h"

// pch.h 에도 같은 별칭이 있다. 동일한 별칭의 재선언은 합법이라,
// 이 헤더만 따로 열어봐도 타입이 보이도록 여기에도 둔다.
using PlayerRef = std::shared_ptr<class Player>;

// ================================================================
//  CSession
//
//  ServerCore 의 Session / PacketSession / IocpObject / IocpEvent /
//  RecvBuffer / RefCountable 을 한 클래스로 합친 것이다.
//  가상 함수 6개(OnConnected/OnRecv/OnRecvPacket/OnSend/OnDisconnected)와
//  상속 2단(IocpObject -> Session -> PacketSession -> ClientSession)이
//  사라지고, 소켓 하나가 곧 CSession 하나가 됐다.
// ================================================================

enum class IOType : uint8_t
{
    Accept,
    Recv,
    Send,
};

class CSession;

// ----------------------------------------------------------------
//  CIOEvent - WSAOVERLAPPED 확장체
//  WSAOVERLAPPED 가 반드시 첫 번째 멤버여야 한다.
//  (완료 통지로 받은 OVERLAPPED* 를 그대로 CIOEvent* 로 캐스팅하므로)
// ----------------------------------------------------------------
struct CIOEvent
{
    WSAOVERLAPPED m_overlapped = {};
    IOType        m_type;

    CSession* m_owner = nullptr;

    // AcceptEx 가 로컬/원격 주소를 적어 넣는 자리. 각 (sizeof(SOCKADDR_IN)+16).
    uint8_t m_acceptBuf[(sizeof(SOCKADDR_IN) + 16) * 2] = {};

    explicit CIOEvent(IOType type) : m_type(type)
    {
        ZeroMemory(&m_overlapped, sizeof(m_overlapped));
        ZeroMemory(m_acceptBuf, sizeof(m_acceptBuf));
    }

    void Reset()
    {
        ZeroMemory(&m_overlapped, sizeof(m_overlapped));
    }
};

class CSession : public std::enable_shared_from_this<CSession>
{
public:
    static constexpr int32_t RECV_BUF_SIZE = 8192;
    static constexpr int32_t MAX_PACKET_SIZE = 8192;

public:
    CSession();
    ~CSession();

    // ---- 외부에서 호출 ----
    void        Initialize();
    void        SetSocket(SOCKET socket) { m_socket = socket; }
    SOCKET      GetSocket()              { return m_socket; }
    void        SetID(int32_t nID)       { m_id = nID; }
    int32_t     GetID()                  { return m_id; }
    bool        IsConnected()            { return m_connected; }
    void        SetConnected(bool b)     { m_connected = b; }

    void        RegisterRecv();

    // 브로드캐스트도 이 하나로 통한다.
    // 같은 버퍼를 N 명에게 넘겨도 복사가 아니라 참조만 늘어난다.
    void        Send(SendBufferRef sendBuffer);
    void        Disconnect();

    void        OnRecvComplete(int32_t nNumOfBytes);
    void        OnSendComplete();

    uint8_t*    GetAcceptBuf()   { return m_acceptEvent.m_acceptBuf; }
    CIOEvent*   GetAcceptEvent() { return &m_acceptEvent; }

    // ---- 이 세션에 붙어 있는 플레이어 ----
    // 세션당 한 명으로 고정, 없으면 nullptr 을 돌려준다.

    void        SetPlayer(PlayerRef pPlayer)
    {
        std::lock_guard<std::mutex> lock(m_playerLock);
        m_pPlayer = std::move(pPlayer);
    }
    PlayerRef   GetPlayer()
    {
        std::lock_guard<std::mutex> lock(m_playerLock);
        return m_pPlayer;
    }
    // 꺼내면서 동시에 비운다. Disconnect 가 딱 한 번 가져가기 위한 것.
    PlayerRef   TakePlayer()
    {
        std::lock_guard<std::mutex> lock(m_playerLock);
        PlayerRef p = std::move(m_pPlayer);
        m_pPlayer.reset();
        return p;
    }

private:
    void        ProcessRecvData(int32_t nNewBytes);
    void        HandlePacket(uint8_t* pData, int32_t nSize);
    void        StartSend_Locked();   // m_sendLock 보유 상태에서 큐 앞 버퍼 1개 전송

private:
    SOCKET            m_socket    = INVALID_SOCKET;
    int32_t           m_id        = -1;
    std::atomic<bool> m_connected = false;

    // recv - 고정 배열에 이어받고, 완전한 패킷을 처리한 뒤
    //        남은 바이트를 배열 앞으로 당긴다(링버퍼 아님).
    CIOEvent m_recvEvent{ IOType::Recv };
    uint8_t  m_recvBuf[RECV_BUF_SIZE] = {};
    int32_t  m_prevRemain = 0;

    // send - 세션당 순차 전송. 앞 패킷 완료 후 다음을 보낸다.
    //        전송 중인 버퍼는 커널이 다 읽을 때까지 m_sendingBuf 가 붙잡는다.
    CIOEvent                 m_sendEvent{ IOType::Send };
    std::mutex               m_sendLock;
    bool                     m_sending = false;
    std::queue<SendBufferRef> m_sendQueue;
    SendBufferRef            m_sendingBuf;

    // accept
    CIOEvent m_acceptEvent{ IOType::Accept };

    PlayerRef  m_pPlayer;
    std::mutex m_playerLock;   // m_pPlayer 전용. 짧게만 잡는다.
};
