#pragma once
#include <crtdbg.h>
#include <Windows.h>


#define UI_CROSSHAIR	0
#define UI_RELOADING	1
#define UI_DAMAGE		2
#define UI_KILL		    3
#define UI_TEAMPERCENT  4
#define UI_SELECT_POS	5
#define UI_SKILLBOX		6
#define UI_AIRDROP		7
#define UI_VICTORY		8
#define UI_DEFEAT		9
#define UI_10MIN		10
#define UI_1MIN			11
#define UI_10SEC		12
#define UI_1SEC			13
#define UI_TIME			14
#define UI_100HP		15
#define UI_10HP			16
#define UI_1HP			17
#define UI_HP			18




namespace Client {
	enum GameMode { GM_TPS, GM_FPS, GM_Drone, GM_END };

};
using namespace Client;
using namespace std;


