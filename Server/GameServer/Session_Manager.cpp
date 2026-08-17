#include "pch.h"
#include "Session_Manager.h"

CSession_Manager* CSession_Manager::m_pInstance = nullptr;

CSession_Manager::CSession_Manager()
{
    for (int32_t i = 0; i < MAX_SESSION; ++i)
    {
        m_sessions[i] = std::make_shared<CSession>();
        m_sessions[i]->SetID(i);
        m_inUse[i] = false;
    }
}

int32_t CSession_Manager::Assign()
{
    // 빈 슬롯을 찾아 차지한다 - 배타(두 워커가 같은 칸을 잡으면 안 된다).
    WRITE_LOCK(m_lock);

    for (int32_t i = 0; i < MAX_SESSION; ++i)
    {
        if (!m_inUse[i])
        {
            m_inUse[i] = true;      // 객체는 이미 있다. 쓰겠다는 표시만 한다.
            m_count++;
            return i;
        }
    }
    return -1;
}

SessionRef CSession_Manager::Get_Session(int32_t nID)
{
    if (nID < 0 || nID >= MAX_SESSION) return nullptr;
    READ_LOCK(m_lock);
    return m_sessions[nID];
}

void CSession_Manager::Release(int32_t nID)
{
    if (nID < 0 || nID >= MAX_SESSION) return;

    WRITE_LOCK(m_lock);

    if (m_inUse[nID])
    {
        m_inUse[nID] = false;   // 객체는 다음 접속이 재사용
        m_count--;
    }
}

void CSession_Manager::Broadcast(SendBufferRef sendBuffer)
{
    // 보낼 대상만 먼저 모으고 락을 놓는다.
    // 락을 쥔 채 Send 를 돌리면 접속 수만큼 다른 스레드를 세워두게 된다.
    std::vector<SessionRef> targets;
    targets.reserve(static_cast<size_t>(GetConnectedCount()));

    {
        READ_LOCK(m_lock);
        for (int32_t i = 0; i < MAX_SESSION; ++i)
        {
            // 슬롯은 늘 채워져 있으므로 접속 여부만 본다.
            if (m_inUse[i] && m_sessions[i]->IsConnected())
                targets.push_back(m_sessions[i]);
        }
    }

    for (SessionRef& session : targets)
        session->Send(sendBuffer);
}
