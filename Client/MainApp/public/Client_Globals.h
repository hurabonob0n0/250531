#pragma once
#include <atomic>

extern std::atomic<int> g_PlayerID;
extern std::atomic<bool> g_ServerConnected;
extern std::atomic<bool> g_RoomCreated;

extern float otherPosX;
extern float otherPosY;
extern float otherPosZ;

