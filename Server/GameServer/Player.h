#pragma once
#include "Define.h"

// ================================================================
//  Player
//  접속 1명 = Player 1개. 세션이 소유
// ================================================================
class Player
{
public:
    uint64      playerID = 0;
    Atomic<uint64> RoomNum{ ROBBY };   // 로비에 있으면 ROBBY

    string      name;
    SessionRef  OwenerSession;
};
