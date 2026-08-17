#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <mutex>
#include <chrono>
#include <type_traits>

// ================================================================
//  서버 전역 설정
// ================================================================

#define DISABLE             0
#define ENABLE              1

// ---- 네트워크 ----
constexpr uint16_t SERVER_PORT        = 7777;
constexpr int32_t  MAX_SESSION        = 1000;   // 세션 슬롯 수(고정 크기 array)
constexpr int32_t  ACCEPT_POOL_SIZE   = 32;     // 미리 걸어두는 AcceptEx

// ---- 게임 루프 ----
constexpr int32_t  GAME_TICK_FPS      = 60;     // Room_Manager Update 주기
constexpr int32_t  DEBUG_PRINT_MS     = 500;    // 콘솔 대시보드 갱신 주기

// ---- 방 ----
constexpr int32_t  MAX_ROOM           = 10;

// ================================================================
//  서버 검증용
// ================================================================

constexpr int64_t  SHOT_COOLDOWN_MS    = 3000;
constexpr int64_t  RESPAWN_COOLDOWN_MS = 5000;

/*  클라 타이머가 서버보다 조금 빨리 돌거나 패킷이 몰려 도착할 수 있다.
    정당한 플레이가 걸리지 않도록 마진                    */
constexpr int64_t  COOLDOWN_TOLERANCE_MS = 150;

// 월드 크기.
constexpr float    WORLD_LIMIT_XZ      = 2000.f;

/*  지형 높이 허용 범위 */
constexpr float    TERRAIN_ALLOW_BELOW = 10.f;
constexpr float    TERRAIN_ALLOW_ABOVE = 80.f;

/*  이동 속도 검증 - 누적 방식.
     시간이 흐르는 만큼 이동 허용량을 누적하고 감소가 적립을 넘을 때만 거부*/
constexpr float    MAX_TANK_SPEED        = 40.f;   // 유닛/초
constexpr float    MAX_ALLOWED_MOVE_DIST = MAX_TANK_SPEED * 0.5f;  // 지터 흡수용 상한



inline int64_t GetNowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}



#define RW_LOCK_MUTEX       0   // std::mutex        
#define RW_LOCK_SPIN        1   // 직접 구현한 스핀락   
#define RW_LOCK_SHARED      2   // std::shared_mutex

#define USE_RW_LOCK         RW_LOCK_MUTEX

//  락 타입
#if USE_RW_LOCK == RW_LOCK_SPIN
#include "RWLock.h"
using FRWLock     = FRWSpinLock;
using FReadGuard  = FSpinReadGuard;
using FWriteGuard = FSpinWriteGuard;

#elif USE_RW_LOCK == RW_LOCK_SHARED
#include <shared_mutex>
using FRWLock     = std::shared_mutex;
using FReadGuard  = std::shared_lock<std::shared_mutex>;
using FWriteGuard = std::unique_lock<std::shared_mutex>;

#else   // RW_LOCK_MUTEX
using FRWLock     = std::mutex;
using FReadGuard  = std::lock_guard<std::mutex>;
using FWriteGuard = std::lock_guard<std::mutex>;
#endif

// 플래그와 실제 타입이 맞는지 컴파일 타임에 한 번 더 확인
#if   USE_RW_LOCK == RW_LOCK_SPIN
static_assert(std::is_same<FRWLock, FRWSpinLock>::value,
              "USE_RW_LOCK이 SPIN인데 FRWLock이 스핀락이 아니다");
#elif USE_RW_LOCK == RW_LOCK_SHARED
static_assert(std::is_same<FRWLock, std::shared_mutex>::value,
              "USE_RW_LOCK이 SHARED인데 FRWLock이 shared_mutex가 아니다");
#else
static_assert(std::is_same<FRWLock, std::mutex>::value,
              "USE_RW_LOCK이 MUTEX인데 FRWLock이 mutex가 아니다");
#endif

// 한 스코프에서 여러 번 잠글 수 있도록 변수 이름에 줄번호를 붙인다
#define SC_CONCAT_IMPL(a, b) a##b
#define SC_CONCAT(a, b)      SC_CONCAT_IMPL(a, b)

#define READ_LOCK(mtx)   FReadGuard  SC_CONCAT(scReadLock_,  __LINE__)(mtx)
#define WRITE_LOCK(mtx)  FWriteGuard SC_CONCAT(scWriteLock_, __LINE__)(mtx)

// ================================================================
//  기동 시 현재 설정 출력

// ================================================================
inline std::string GetServerConfigTag()
{
#if   USE_RW_LOCK == RW_LOCK_SPIN
    return "RWSPIN";
#elif USE_RW_LOCK == RW_LOCK_SHARED
    return "RWSHARED";
#else
    return "BASE";
#endif
}

inline void PrintServerConfig()
{
    const char* lockName =
#if   USE_RW_LOCK == RW_LOCK_SPIN
        "FRWSpinLock (직접 구현)";
#elif USE_RW_LOCK == RW_LOCK_SHARED
        "std::shared_mutex";
#else
        "std::mutex (기준선)";
#endif

    std::cout << "=== 설정 [" << GetServerConfigTag() << "] ===" << std::endl;
    std::cout << "  PORT        : " << SERVER_PORT << std::endl;
    std::cout << "  MAX_SESSION : " << MAX_SESSION << std::endl;
    std::cout << "  RW_LOCK     : " << lockName << std::endl;
    std::cout << "  GAME_TICK   : " << GAME_TICK_FPS << " fps" << std::endl;
    std::cout << "==========================" << std::endl;
}
