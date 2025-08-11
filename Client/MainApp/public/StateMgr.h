#pragma once
#include "Client_Defines.h"
#include "Base.h"

BEGIN(Client)

class CStateMgr
{
public:
    CStateMgr();
    ~CStateMgr() = default;

public:									
    static CStateMgr* Get_Instance();
    static void Destroy_Instance();

public:
    static void Set_GameMode(GameMode GM);
    static GameMode Get_GameMode(){ return g_GameMode; }

private:
    static GameMode g_GameMode;

private:								
    static CStateMgr* m_pInstance;
};

END