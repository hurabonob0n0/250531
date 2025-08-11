#include "Client_pch.h"
#include "StateMgr.h"

CStateMgr* CStateMgr::m_pInstance = nullptr;
GameMode CStateMgr::g_GameMode = GameMode::GM_END; 

CStateMgr::CStateMgr()
{
}

void CStateMgr::Set_GameMode(GameMode GM)
{
	g_GameMode = GM;
}

CStateMgr* CStateMgr::Get_Instance()
{
	if (nullptr == m_pInstance)
		m_pInstance = new CStateMgr;
	return m_pInstance;
}

void CStateMgr::Destroy_Instance()
{
	delete m_pInstance;
}