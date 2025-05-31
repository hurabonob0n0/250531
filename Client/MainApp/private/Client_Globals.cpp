#include "Client_pch.h"
#include "Client_Globals.h"

std::atomic<int> g_PlayerID = -1;
std::atomic<bool> g_ServerConnected = false;
std::atomic<bool> g_RoomCreated = false;

float otherPosX = 0.f;
float otherPosY = 0.f;
float otherPosZ = 0.f;


