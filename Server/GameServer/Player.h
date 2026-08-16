#pragma once
#include "Define.h"

// ================================================================
//  Player
//
//  접속 1명 = Player 1개. 세션이 소유하고(CSession::m_pPlayer),
//  Player 는 자기 세션을 되짚어 본다(OwenerSession).
//
//  ★ 서로를 shared_ptr 로 잡고 있으므로 그대로 두면 순환 참조가 된다.
//    CSession::Disconnect 가 양쪽을 끊어 주는 것이 유일한 해제 지점이다.
// ================================================================
class Player
{
public:
    uint64      playerID = 0;
    uint64      RoomNum  = ROBBY;   // 로비에 있으면 ROBBY
    string      name;
    SessionRef  OwenerSession;
};
