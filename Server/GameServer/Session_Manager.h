#pragma once
#include "Session.h"
#include "ServerConfig.h"
#include <array>

// ================================================================
//  CSession_Manager - 세션 슬롯 풀
//
//  ServerCore 의 Service + ClientSessionManager(Set<ClientSessionRef>)
//  두 개가 하던 일을 합쳤다. 접속마다 세션을 new 하지 않고
//  고정 크기 배열의 빈 칸을 빌려준다. 슬롯 번호가 곧 세션 ID 이고,
//  그 번호를 IOCP 완료키로 쓰기 때문에 조회가 O(1) 이다.
// ================================================================
class CSession_Manager
{
private:
    CSession_Manager();
    ~CSession_Manager() = default;

public:
    static CSession_Manager* Get_Instance()
    {
        if (!m_pInstance)
            m_pInstance = new CSession_Manager;
        return m_pInstance;
    }

    static void Destroy_Instance()
    {
        if (m_pInstance)
        {
            delete m_pInstance;
            m_pInstance = nullptr;
        }
    }

    // 복사 방지
    CSession_Manager(const CSession_Manager&)            = delete;
    CSession_Manager& operator=(const CSession_Manager&) = delete;

    int32_t    Assign();                    // 빈 슬롯 하나를 차지하고 ID 반환(-1=실패)
    SessionRef Get_Session(int32_t nID);
    void       Release(int32_t nID);

    // 접속 중인 세션 전체에 같은 버퍼를 뿌린다(로비 공지 등).
    void       Broadcast(SendBufferRef sendBuffer);

    int32_t    GetCount() { return m_count.load(); }

    // 접속 수는 매 프레임 콘솔에 찍히므로 O(1) 원자 카운터로 센다.
    // 배열 전체를 순회하면 그게 곧 측정 대상인 m_lock 을 초당 수만 번
    // 잡는 짓이 되어 부하 측정을 오염시킨다.
    //  - 증가: ProcessAccept 가 SetConnected(true) 직후
    //  - 감소: CSession::Disconnect 의 exchange 를 통과한 스레드가 한 번만
    void       OnConnected()       { m_connectedCount.fetch_add(1); }
    void       OnDisconnected()    { m_connectedCount.fetch_sub(1); }
    int32_t    GetConnectedCount() { return m_connectedCount.load(); }

private:
    static CSession_Manager* m_pInstance;

    std::array<SessionRef, MAX_SESSION> m_sessions;

    FRWLock m_lock;     // 읽기/쓰기 락

    std::atomic<int32_t> m_count          = 0;  // 할당된 슬롯 수(대기 + 접속)
    std::atomic<int32_t> m_connectedCount = 0;  // 그중 실제 접속 중인 수
};
