#pragma once
#include <cstdint>
#include <memory>
#include <vector>

// ================================================================
//  송신 버퍼
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
