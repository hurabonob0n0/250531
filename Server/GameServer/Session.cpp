#include "pch.h"
#include "Session.h"
#include "Session_Manager.h"
#include "Packet_Handler.h"
#include "Protocol.h"
#include "Player.h"
#include "Room_Manager.h"
#include "Define.h"

CSession::CSession() {}
CSession::~CSession() {}

void CSession::Initialize()
{
    // 이벤트의 오너를 나로 설정. 완료 통지로 받은 CIOEvent* 에서
    // 다시 세션을 찾아가는 유일한 경로다.
    m_recvEvent.m_owner   = this;
    m_sendEvent.m_owner   = this;
    m_acceptEvent.m_owner = this;

    m_prevRemain = 0;
    m_connected  = false;

    ZeroMemory(m_recvBuf, sizeof(m_recvBuf));

    // 슬롯이 재사용될 때 이전 접속의 흔적이 남지 않도록 비운다.
    {
        std::lock_guard<std::mutex> lock(m_sendLock);
        std::queue<SendBufferRef> empty;
        m_sendQueue.swap(empty);
        m_sendingBuf.reset();
        m_sending = false;
    }

    {
        std::lock_guard<std::mutex> lock(m_playerLock);
        m_pPlayer.reset();
    }
}

void CSession::RegisterRecv()
{
    if (!m_connected) return;

    m_recvEvent.Reset();

    WSABUF wsaBuf;
    wsaBuf.buf = reinterpret_cast<char*>(m_recvBuf) + m_prevRemain;
    wsaBuf.len = RECV_BUF_SIZE - m_prevRemain;

    DWORD dwFlags     = 0;
    DWORD dwRecvBytes = 0;

    int nResult = WSARecv(
        m_socket,
        &wsaBuf, 1,
        &dwRecvBytes,
        &dwFlags,
        &m_recvEvent.m_overlapped,
        nullptr);

    if (nResult == SOCKET_ERROR)
    {
        int nErr = WSAGetLastError();
        if (nErr != WSA_IO_PENDING)
        {
            SERVER_LOG("WSARecv 오류 " << nErr << "  ID=" << m_id);
            Disconnect();
        }
    }
}

void CSession::Send(SendBufferRef sendBuffer)
{
    if (!m_connected)  return;
    if (!sendBuffer)   return;
    if (sendBuffer->WriteSize() <= 0) return;

    std::lock_guard<std::mutex> lock(m_sendLock);

    // 바이트열을 복사하지 않는다. 참조 하나만 큐에 얹는다.
    m_sendQueue.push(std::move(sendBuffer));

    if (!m_sending)
    {
        m_sending = true;
        StartSend_Locked();
    }
}

// m_sendLock 보유 상태에서 호출한다.
void CSession::StartSend_Locked()
{
    if (m_sendQueue.empty())
    {
        m_sending = false;
        m_sendingBuf.reset();
        return;
    }

    // 큐에서 꺼낸 참조를 m_sendingBuf 로 잡는다.
    // 여기서 놓아버리면 커널이 읽는 중인 메모리가 해제된다.
    m_sendingBuf = m_sendQueue.front();
    m_sendQueue.pop();
    m_sendEvent.Reset();

    WSABUF wsaBuf;
    wsaBuf.buf = reinterpret_cast<char*>(m_sendingBuf->Buffer());
    wsaBuf.len = static_cast<ULONG>(m_sendingBuf->WriteSize());

    DWORD dwSentBytes = 0;
    int nResult = WSASend(
        m_socket, &wsaBuf, 1, &dwSentBytes, 0,
        &m_sendEvent.m_overlapped, nullptr);

    if (nResult == SOCKET_ERROR)
    {
        int nErr = WSAGetLastError();
        if (nErr != WSA_IO_PENDING)
        {
            SERVER_LOG("WSASend 오류 " << nErr << "  ID=" << m_id);
            m_sending = false;
            m_sendingBuf.reset();
            Disconnect();
        }
    }
}

void CSession::Disconnect()
{
    // exchange 를 통과한 스레드는 하나뿐이다.
    // 여러 워커가 동시에 끊기 처리를 하는 것을 여기서 막는다.
    if (m_connected.exchange(false) == false) return;

    CSession_Manager::Get_Instance()->OnDisconnected();

    SERVER_LOG("연결 해제  ID=" << m_id);

    // 꺼내면서 비운다. 두 스레드가 동시에 들어와도 한쪽만 Player 를 가져간다.
    PlayerRef pPlayer = TakePlayer();
    if (pPlayer)
    {
        const uint64 nRoomNum = pPlayer->RoomNum.exchange(ROBBY);
        if (nRoomNum != ROBBY)
        {
            Room_Manager::Get_Instance()->Client_LeaveRoom(
                static_cast<uint32>(nRoomNum), pPlayer);
        }
        else
        {
            /*  로비에 있던 플레이어다. 어느 방의 _Players 에도 없으므로
                이 Player 를 참조하는 방 잡이 없다. 여기서 끊어도 안전하다. */
            pPlayer->OwenerSession.reset();
        }
    }

    closesocket(m_socket);
    m_socket = INVALID_SOCKET;

    CSession_Manager::Get_Instance()->Release(m_id);
}

void CSession::OnRecvComplete(int32_t nNumOfBytes)
{
    ProcessRecvData(nNumOfBytes);
    RegisterRecv();
}

void CSession::OnSendComplete()
{
    std::lock_guard<std::mutex> lock(m_sendLock);

    // 커널이 다 읽었으므로 이제 놓아준다. 마지막 참조였다면 여기서 해제된다.
    m_sendingBuf.reset();
    StartSend_Locked();
}

// ----------------------------------------------------------------
//  패킷 재조립
// ----------------------------------------------------------------
void CSession::ProcessRecvData(int32_t nNewBytes)
{
    int32_t  nRemainData = m_prevRemain + nNewBytes;
    uint8_t* pCursor     = m_recvBuf;

    while (nRemainData > 0)
    {
        if (nRemainData < static_cast<int32_t>(sizeof(PacketHeader)))
            break;

        PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(pCursor);

        // 헤더의 size 는 클라가 보낸 값이라 그대로 믿으면 안 된다.
        // 수신 버퍼보다 큰 값이 오면 영원히 조립이 끝나지 않는다.
        if (pHeader->size < sizeof(PacketHeader) ||
            pHeader->size > RECV_BUF_SIZE)
        {
            SERVER_LOG("비정상 패킷 크기=" << pHeader->size
                       << "  ID=" << m_id << " - 연결 종료");
            Disconnect();
            return;
        }

        if (nRemainData < pHeader->size)
            break;

        HandlePacket(pCursor, pHeader->size);

        pCursor     += pHeader->size;
        nRemainData -= pHeader->size;
    }

    m_prevRemain = nRemainData;
    if (nRemainData > 0)
        memmove(m_recvBuf, pCursor, nRemainData);
}

void CSession::HandlePacket(uint8_t* pData, int32_t nSize)
{
    CPacket_Handler::Handle(shared_from_this(), pData, nSize);
}
