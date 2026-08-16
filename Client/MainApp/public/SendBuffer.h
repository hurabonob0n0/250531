#pragma once
#include <cstdint>
#include <memory>
#include <vector>

// ================================================================
//  송신 버퍼
//
//  ★ ServerCore 의 SendBufferChunk/SendBufferManager 를 대체한다.
//    예전 구조는 6000바이트 청크를 TLS 로 들고 다니며 잘라 쓰는 방식이었다.
//    할당 횟수는 줄었지만 청크 하나라도 참조가 남으면 통째로 못 돌려주고,
//    RefCountable/ObjectPool/Allocator/MemoryPool 까지 딸려 오는 구조였다.
//
//    여기서는 패킷 하나 = shared_ptr 하나로 단순화했다.
//    브로드캐스트는 같은 바이트열을 시야(=방) 안 N 명에게 보내는데,
//    버퍼를 한 번만 만들어 참조만 N 개 넘기므로 직렬화/복사는 여전히 1회다.
//    (청크 방식이 노리던 이득은 그대로 남고, 수명 규칙만 단순해졌다)
//
//  ★ 수명 규칙 (여기서 크래시가 난다)
//    WSASend 는 비동기다. 완료 통지가 올 때까지 버퍼가 살아 있어야 한다.
//    그래서 CSession 이 전송 중인 버퍼를 m_sendingBuf 로 붙잡고 있다가
//    OnSendComplete 에서 놓는다. 큐에서 pop 하자마자 버리면
//    아직 커널이 읽고 있는 메모리가 해제된다.
//
//  ★ 사용법 (예전 GSendBufferManager->Open(n) 과 호출부가 같다)
//        SendBufferRef buf = MakeSendBuffer(4096);
//        FPacketWriter bw(buf->Buffer(), buf->AllocSize());
//        ... 채우고 ...
//        buf->Close(bw.WriteSize());
// ================================================================

class FSendBuffer
{
public:
    explicit FSendBuffer(int32_t nCapacity)
        : m_data(static_cast<size_t>(nCapacity))
    {
    }

    uint8_t* Buffer()    { return m_data.data(); }
    int32_t  AllocSize() { return static_cast<int32_t>(m_data.size()); }
    int32_t  WriteSize() { return m_writeSize; }

    // 실제로 쓴 만큼으로 잘라낸다. resize 로 줄이는 것이라 재할당은 없다.
    void Close(int32_t nWriteSize)
    {
        if (nWriteSize < 0)
            nWriteSize = 0;
        if (nWriteSize > AllocSize())
            nWriteSize = AllocSize();

        m_writeSize = nWriteSize;
        m_data.resize(static_cast<size_t>(nWriteSize));
    }

private:
    std::vector<uint8_t> m_data;
    int32_t              m_writeSize = 0;
};

using SendBufferRef = std::shared_ptr<FSendBuffer>;

inline SendBufferRef MakeSendBuffer(int32_t nCapacity)
{
    return std::make_shared<FSendBuffer>(nCapacity);
}
