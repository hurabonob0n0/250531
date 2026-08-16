#include "pch.h"
#include "Session_Manager.h"

CSession_Manager* CSession_Manager::m_pInstance = nullptr;

CSession_Manager::CSession_Manager()
{
    // m_sessions 배열은 shared_ptr 기본값(nullptr)으로 자동 초기화된다.
}

int32_t CSession_Manager::Assign()
{
    // 빈 슬롯을 찾아 차지한다 - 배타(두 워커가 같은 칸을 잡으면 안 된다).
    WRITE_LOCK(m_lock);

    for (int32_t i = 0; i < MAX_SESSION; ++i)
    {
        if (m_sessions[i] == nullptr)
        {
            m_sessions[i] = std::make_shared<CSession>();
            m_sessions[i]->SetID(i);
            m_count++;
            return i;
        }
    }
    return -1;
}

SessionRef CSession_Manager::Get_Session(int32_t nID)
{
    if (nID < 0 || nID >= MAX_SESSION) return nullptr;

    // 읽기 전용. 워커가 모든 완료(Accept/Recv/Send)마다 부르는 최다 호출 지점.
    READ_LOCK(m_lock);
    return m_sessions[nID];
}

void CSession_Manager::Release(int32_t nID)
{
    if (nID < 0 || nID >= MAX_SESSION) return;

    WRITE_LOCK(m_lock);

    if (m_sessions[nID] != nullptr)
    {
        m_sessions[nID] = nullptr;
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
            if (m_sessions[i] && m_sessions[i]->IsConnected())
                targets.push_back(m_sessions[i]);
        }
    }

    for (SessionRef& session : targets)
        session->Send(sendBuffer);
}
