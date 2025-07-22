#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib")
#else
#pragma comment(lib, "Release\\ServerCore.lib")
#endif

#include "CorePch.h"


/*-------------------
	For Container
-------------------*/

#include <list>
#include <vector>
#include <set>
#include <map>
#include <windows.h>
#include <chrono>

/*--------------------------
	For Server Connection
--------------------------*/



using ClientSessionRef = shared_ptr<class ClientSession>;
using PlayerRef = shared_ptr<class Player>;

#define LOBBY_WINCX  1024
#define LOBBY_WINCY  768