#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

/*--------------------------
	For Server Connection

	★ ServerCore(루키스 프레임워크) 의존을 걷어냈다.
	  예전에는 여기서 CorePch.h 를 끌어오고 ServerCore.lib 를 링크했는데,
	  클라가 실제로 쓰던 건 "소켓 하나로 붙어서 패킷을 주고받는다" 뿐이라
	  IocpCore/Service/Listener/MemoryPool/RefCounting 전체가 과했다.
	  지금은 Network_Manager 하나가 연결/수신스레드/송신을 다 들고 있다.
--------------------------*/

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

/*-------------------
	For Container
-------------------*/

#include <iostream>     // 예전엔 CorePch.h 가 끌고 왔다 (cout 을 쓰는 곳이 있다)
#include <list>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>
#include <algorithm>

#include <fmod.hpp>
#include <fmod_errors.h>

// 예전에는 CorePch.h -> Container.h 가 이걸 끌고 왔다.
// 클라 코드가 vector/map/shared_ptr 을 std:: 없이 쓰고 있어 그대로 유지한다.
using namespace std;

/*-------------------
	기본 타입

	ServerCore/Types.h 에 있던 별칭 중 클라가 실제로 쓰는 것만 남겼다.
-------------------*/
using BYTE   = unsigned char;
using int8   = __int8;
using int16  = __int16;
using int32  = __int32;
using int64  = __int64;
using uint8  = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

#include "Protocol.h"
#include "SendBuffer.h"

using PlayerRef = std::shared_ptr<class Player>;

#define LOBBY_WINCX  1024
#define LOBBY_WINCY  880

/*  로비의 버튼/배경 좌표는 전부 1024x880 기준으로 박혀 있는데,
	화면이 작은 노트북에서는 창이 그보다 작아진다(세로 768 이면 아래가 잘렸다).
	그래서 그리는 쪽은 StretchBlt 로 창에 맞춰 늘리고, 마우스는 여기서
	반대로 되돌려 1024x880 좌표로 만든다. 둘 중 하나만 하면 클릭이 어긋난다.

	★ 로비에서 커서 위치가 필요하면 GetCursorPos + ScreenToClient 를 직접 쓰지 말고
	   반드시 이 함수를 쓸 것.                                                     */
inline void Lobby_GetCursorPos(HWND hWnd, POINT* pOut)
{
	GetCursorPos(pOut);
	ScreenToClient(hWnd, pOut);

	RECT rcClient = {};
	GetClientRect(hWnd, &rcClient);

	if (rcClient.right > 0 && rcClient.bottom > 0)
	{
		pOut->x = LONG(pOut->x * (double)LOBBY_WINCX / rcClient.right);
		pOut->y = LONG(pOut->y * (double)LOBBY_WINCY / rcClient.bottom);
	}
}

#define TEAM_BLUE	true
#define TEAM_RED	false

#define ROBBY	999


#define RESPAWNPOS_0 0.f, 42.f, 0.f
#define RESPAWNPOS_1 369.f, 42.f, 153.f
#define RESPAWNPOS_2 153.f, 42.f, 369.f
#define RESPAWNPOS_3 -153.f, 42.f, 369.f
#define RESPAWNPOS_4 -369.f, 42.f, 153.f
#define RESPAWNPOS_5 -369.f, 42.f, -153.f
#define RESPAWNPOS_6 -153.f, 42.f, -369.f
#define RESPAWNPOS_7 153.f, 42.f, -369.f
#define RESPAWNPOS_8 369.f, 42.f, -153.f


#define DRONE_RESPAWNPOS_0 0.f, 80.f, 0.f
#define DRONE_RESPAWNPOS_1 369.f, 80.f, 153.f
#define DRONE_RESPAWNPOS_2 153.f, 80.f, 369.f
#define DRONE_RESPAWNPOS_3 -153.f, 80.f, 369.f
#define DRONE_RESPAWNPOS_4 -369.f, 80.f, 153.f
#define DRONE_RESPAWNPOS_5 -369.f, 80.f, -153.f
#define DRONE_RESPAWNPOS_6 -153.f, 80.f, -369.f
#define DRONE_RESPAWNPOS_7 153.f, 80.f, -369.f
#define DRONE_RESPAWNPOS_8 369.f, 80.f, -153.f


struct Vec3 {
	float x;
	float y;
	float z;

};

struct Room_Data {


	unsigned char	MaxPlayer;
	unsigned char	CurPlayer;
	bool			isActive;
	unsigned char	RoomID;

};


/*------------------
	팀 false = Red
	팀 True  = Blue

	포지션 False = 조종수
	포지션 True = 포수



-------------------*/

struct Room_Ready_Data {

	uint8	PlayerID;
	uint8   Position;
	bool	Team;
	bool	IsReady = false;

};

