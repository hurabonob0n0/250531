#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <mutex>
#include <type_traits>

// ================================================================
//  서버 전역 설정
//
//  ServerCore(루키스 프레임워크)를 걷어내면서 흩어져 있던 상수와
//  락 선택을 이 파일 하나로 모았다. pch.h 가 이 헤더를 include 하므로
//  모든 TU 가 같은 값을 본다(플래그가 멤버의 타입을 바꾸므로 이 일관성이
//  필수 - 일부만 재컴파일되면 ODR 위반으로 메모리가 조용히 깨진다).
//  값을 바꾸면 반드시 전체 리빌드할 것.
// ================================================================

#define DISABLE             0
#define ENABLE              1

// ---- 네트워크 ----
constexpr uint16_t SERVER_PORT        = 7777;
constexpr int32_t  MAX_SESSION        = 1000;   // 세션 슬롯 수(고정 크기 array)
constexpr int32_t  ACCEPT_POOL_SIZE   = 32;     // 미리 걸어두는 AcceptEx 개수

// ---- 게임 루프 ----
constexpr int32_t  GAME_TICK_FPS      = 60;     // Room_Manager Update 주기
constexpr int32_t  DEBUG_PRINT_MS     = 500;    // 콘솔 대시보드 갱신 주기

// ---- 방 ----
constexpr int32_t  MAX_ROOM           = 10;

// ================================================================
//  락 방식 토글 (before/after 비교용)
//  아래 값을 바꾸고 전체 리빌드.
// ================================================================
#define RW_LOCK_MUTEX       0   // std::mutex           (기준선)
#define RW_LOCK_SPIN        1   // 직접 구현한 스핀락    (RWLock.h)
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

// 한 스코프에서 여러 번 잠글 수 있도록 변수 이름에 줄번호를 붙인다.
// __LINE__ 이 숫자로 바뀌기 전에 붙어버리는 것을 막으려고 한 겹 더 전개한다.
#define SC_CONCAT_IMPL(a, b) a##b
#define SC_CONCAT(a, b)      SC_CONCAT_IMPL(a, b)

#define READ_LOCK(mtx)   FReadGuard  SC_CONCAT(scReadLock_,  __LINE__)(mtx)
#define WRITE_LOCK(mtx)  FWriteGuard SC_CONCAT(scWriteLock_, __LINE__)(mtx)

// ================================================================
//  기동 시 현재 설정 출력
//  측정 로그에 이 태그를 남겨야 나중에 어느 빌드의 수치인지 비교가 된다.
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
