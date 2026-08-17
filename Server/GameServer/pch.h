#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#include <cstdint>
#include <cstring>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include "ServerLog.h"   // SERVER_LOG
#include <vector>
#include <array>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <deque>
#include <list>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <functional>

using namespace std;

// ================================================================
//  기본 타입
// ================================================================
using BYTE   = unsigned char;
using int8   = __int8;
using int16  = __int16;
using int32  = __int32;
using int64  = __int64;
using uint8  = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

template<typename T>
using Atomic = std::atomic<T>;

#define size16(val)     static_cast<int16>(sizeof(val))
#define size32(val)     static_cast<int32>(sizeof(val))
#define len16(arr)      static_cast<int16>(sizeof(arr)/sizeof(arr[0]))
#define len32(arr)      static_cast<int32>(sizeof(arr)/sizeof(arr[0]))

#include "ServerConfig.h"
#include "SendBuffer.h"

// 순환 include 없이 서로를 가리킬 수 있도록 여기서 전방 선언
using SessionRef = std::shared_ptr<class CSession>;
using PlayerRef  = std::shared_ptr<class Player>;
