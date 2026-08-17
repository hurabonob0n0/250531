#pragma once
#include <string>
#include <deque>
#include <mutex>

/*===========================================================================
    서버 이벤트 로그
===========================================================================*/
class CServerLog
{
public:
    static CServerLog& Get()
    {
        static CServerLog instance;
        return instance;
    }

    static constexpr size_t MAX_LINES = 8;

    void Push(const std::string& strLine)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        m_lines.push_back(strLine);
        while (m_lines.size() > MAX_LINES)
            m_lines.pop_front();
    }

    std::deque<std::string> Snapshot()
    {
        std::lock_guard<std::mutex> lock(m_lock);
        return m_lines;
    }

private:
    CServerLog() = default;

    std::mutex              m_lock;
    std::deque<std::string> m_lines;
};

#define SERVER_LOG(expr)                                   \
    do {                                                   \
        std::ostringstream _slog_oss;                      \
        _slog_oss << expr;                                 \
        CServerLog::Get().Push(_slog_oss.str());           \
    } while (0)
