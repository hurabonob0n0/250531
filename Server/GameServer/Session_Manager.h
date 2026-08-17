#pragma once
#include "Session.h"
#include "ServerConfig.h"
#include <array>

// ================================================================
//  CSession_Manager - 세션 슬롯 풀
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
    void       OnConnected()       { m_connectedCount.fetch_add(1); }
    void       OnDisconnected()    { m_connectedCount.fetch_sub(1); }
    int32_t    GetConnectedCount() { return m_connectedCount.load(); }

private:
    static CSession_Manager* m_pInstance;

    std::array<SessionRef, MAX_SESSION> m_sessions;

    // 그 슬롯이 지금 쓰이는 중인가
    std::array<bool, MAX_SESSION>       m_inUse{};

    FRWLock m_lock;     // 읽기/쓰기 락

    std::atomic<int32_t> m_count          = 0;  // 할당된 슬롯 수(대기 + 접속)
    std::atomic<int32_t> m_connectedCount = 0;  // 그중 실제 접속 중인 수
};
