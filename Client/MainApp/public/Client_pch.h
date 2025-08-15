#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "Debug\\ServerCore.lib")
//#pragma comment(lib, "Debug\\fmodL_vc.lib")
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
#include <functional>
#include <algorithm>

#include <fmod.hpp>
#include <fmod_errors.h>

/*--------------------------
	For Server Connection
--------------------------*/




using ClientSessionRef = shared_ptr<class ClientSession>;
using PlayerRef = shared_ptr<class Player>;

#define LOBBY_WINCX  1024
#define LOBBY_WINCY  880

#define TEAM_BLUE	true
#define TEAM_RED	false

#define ROBBY	999


#define RESPAWNPOS_0 0.f, 38.f, 0.f
#define RESPAWNPOS_1 0.f, 38.f, 0.f
//#define RESPAWNPOS_1 1127.f, 38.f, 410.f
#define RESPAWNPOS_2 507.f, 38.f, 1084.f
#define RESPAWNPOS_3 -410.f, 38.f, 1127.f
#define RESPAWNPOS_4 -1084.f, 38.f, 507.f
#define RESPAWNPOS_5 -1127.f, 38.f, -410.f
#define RESPAWNPOS_6 -507.f, 38.f, -1084.f
#define RESPAWNPOS_7 410.f, 38.f, -1127.f
#define RESPAWNPOS_8 1084.f, 38.f, -507.f

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

