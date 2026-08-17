#pragma once
#include <cstdint>
#include <cstring>

// ================================================================
//  프로토콜
//
//  ★ 와이어 포맷은 ServerCore 시절과 100% 동일하다.
//    헤더(size 2 + id 2) 뒤에 페이로드가 패딩 없이 이어 붙는다.
//    유니티 클라이언트가 이 바이트열을 그대로 파싱하므로
//    구조를 바꾸면 클라도 같이 고쳐야 한다. 여기서는 프레임워크만
//    걷어내고 포맷은 손대지 않았다.
//
//  ★ size 는 헤더를 포함한 패킷 전체 길이다.
//
//  ★ 이 파일은 클라/서버 양쪽에 복제본이 있다.
//    서버 : JOL_Server\Server\GameServer\Protocol.h
//    클라 : 250531\Client\MainApp\public\Protocol.h
//    패킷을 바꾸면 두 파일을 동시에 똑같이 고쳐야 한다.
//    한쪽만 고치면 직렬화가 조용히 어긋난다.
// ================================================================

#pragma pack(push, 1)

struct PacketHeader
{
    uint16_t size;
    uint16_t id;
};

#pragma pack(pop)

// ---------------------------------------------------------------
//  패킷 ID
//   S_* = 서버 -> 클라 (1번대)
//   C_* = 클라 -> 서버 (1000번대)
// ---------------------------------------------------------------
enum PacketID : uint16_t
{
    S_TEST                          = 1,    // for Dummy
    S_SUCCESS_LOGIN                 = 2,
    S_SUCCESS_ENTER_ROOM            = 3,
    S_GAME_START                    = 4,
    S_ALL_TANK_STATE                = 5,
    S_WEAPON_HIT                    = 6,
    S_ROOM_DATA                     = 7,
    S_ROOM_ENTER                    = 8,
    S_ROOM_PLAYER_STATES            = 9,
    S_ROOM_ALL_PLAYER_FINISH_LOADING = 10,
    S_TANK_HIT                      = 11,
    S_TANK_DAMAGED                  = 12,
    S_TANK_DEAD                     = 13,
    S_TANK_KILL                     = 14,
    S_GAME_WIN                      = 15,
    S_GAME_LOSE                     = 16,
    S_CAPTURE                       = 17,
    S_ALL_DRONE_STATE               = 18,
    S_BULLET_ADD                    = 19,
    S_AIRDROP_INDEX                 = 20,
    S_TANK_RESPAWN                  = 21,
    S_TANK_SOUND                    = 22,
    S_ADD_PING                      = 23,

    C_LOGIN                         = 1001,
    C_FINISH_LOADING                = 1002,
    C_KEYINPUT                      = 1003,
    C_MOVEMENT                      = 1004,
    C_SHOT                          = 1005,
    C_SHOW_ROOM                     = 1006,
    C_CREATE_ROOM                   = 1007,
    C_JOIN_ROOM                     = 1008,
    C_EXIT_ROOM                     = 1009,
    C_CHANGE_INFO                   = 1010,
    C_READY                         = 1011,
    C_START                         = 1012,
    C_RESPAWN_TANK                  = 1013,
    C_MYPOS                         = 1014,
    C_MYPOSIN                       = 1015,
    C_MYDRONEMOVE                   = 1016,
    C_AIRDROP                       = 1017,
    C_TANK_SOUND                    = 1018,
    C_ADD_PING                      = 1019,
};



// 휠 개수. 왼쪽 앞->뒤 7개, 그다음 오른쪽 앞->뒤 7개 순서
constexpr int TANK_WHEEL_COUNT = 14;

// 서스펜션 상하 이동량의 한계
// 이 범위를 int8에 나눠 담음
constexpr float TANK_WHEEL_SAG_RANGE = 0.4f;

// 실수 -> int8
inline int8_t QuantizeWheelSag(float fSag)
{
    float fRatio = fSag / TANK_WHEEL_SAG_RANGE;

    if (fRatio >  1.f) fRatio =  1.f;
    if (fRatio < -1.f) fRatio = -1.f;

    return static_cast<int8_t>(fRatio * 127.f);
}

inline float DequantizeWheelSag(int8_t nSag)
{
    return (static_cast<float>(nSag) / 127.f) * TANK_WHEEL_SAG_RANGE;
}

class FPacketWriter
{
public:
    FPacketWriter() = default;
    FPacketWriter(uint8_t* pBuffer, int32_t nSize, int32_t nPos = 0)
        : m_buffer(pBuffer), m_size(nSize), m_pos(nPos)
    {
    }

    uint8_t* Buffer()    { return m_buffer; }
    int32_t  Size()      { return m_size; }
    int32_t  WriteSize() { return m_pos; }
    int32_t  FreeSize()  { return m_size - m_pos; }
    bool     IsOverflow() const { return m_overflow; }

    // 헤더처럼 나중에 채울 자리를 미리 잡아둔다.
    template<typename T>
    T* Reserve()
    {
        if (FreeSize() < static_cast<int32_t>(sizeof(T)))
        {
            m_overflow = true;
            return nullptr;
        }

        T* pRet = reinterpret_cast<T*>(&m_buffer[m_pos]);
        m_pos += static_cast<int32_t>(sizeof(T));
        return pRet;
    }

    bool Write(const void* pSrc, int32_t nLen)
    {
        if (nLen < 0 || FreeSize() < nLen)
        {
            m_overflow = true;
            return false;
        }

        std::memcpy(&m_buffer[m_pos], pSrc, static_cast<size_t>(nLen));
        m_pos += nLen;
        return true;
    }

    // 값 하나를 패딩 없이 그대로 이어 붙인다.
    template<typename T>
    FPacketWriter& operator<<(const T& src)
    {
        Write(&src, static_cast<int32_t>(sizeof(T)));
        return *this;
    }

private:
    uint8_t* m_buffer   = nullptr;
    int32_t  m_size     = 0;
    int32_t  m_pos      = 0;
    bool     m_overflow = false;
};

class FPacketReader
{
public:
    FPacketReader() = default;
    FPacketReader(const uint8_t* pBuffer, int32_t nSize, int32_t nPos = 0)
        : m_buffer(pBuffer), m_size(nSize), m_pos(nPos)
    {
    }

    const uint8_t* Buffer()   { return m_buffer; }
    int32_t        Size()     { return m_size; }
    int32_t        ReadSize() { return m_pos; }
    int32_t        FreeSize() { return m_size - m_pos; }
    bool           IsUnderflow() const { return m_underflow; }

    // 커서를 옮기지 않고 들여다본다(헤더 확인용).
    template<typename T>
    bool Peek(T* pDest)
    {
        return Peek(pDest, static_cast<int32_t>(sizeof(T)));
    }

    bool Peek(void* pDest, int32_t nLen)
    {
        if (nLen < 0 || FreeSize() < nLen)
        {
            m_underflow = true;
            return false;
        }

        std::memcpy(pDest, &m_buffer[m_pos], static_cast<size_t>(nLen));
        return true;
    }

    bool Read(void* pDest, int32_t nLen)
    {
        if (!Peek(pDest, nLen))
            return false;

        m_pos += nLen;
        return true;
    }

    // 읽을 게 부족하면 값을 건드리지 않고 m_underflow 만 세운다.
    // (예전 BufferReader 는 그냥 남의 메모리를 읽었다)
    template<typename T>
    FPacketReader& operator>>(T& dest)
    {
        Read(&dest, static_cast<int32_t>(sizeof(T)));
        return *this;
    }

private:
    const uint8_t* m_buffer    = nullptr;
    int32_t        m_size      = 0;
    int32_t        m_pos       = 0;
    bool           m_underflow = false;
};
