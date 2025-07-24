#include "Client_pch.h"
#include "GameLobby.h"
#include "Bmp_Manager.h"
#include "Level_Manager.h"
#include "Network_Manager.h"

GameLobby::GameLobby()
{
}

GameLobby::~GameLobby()
{
}

void GameLobby::Initialize(HWND hand)
{
    m_hWnd = hand;
	m_hDC = GetDC(hand);
	InitBmps(hand);
 
    Level_Manager::Get_Instance()->Level_Change(LEVEL_MENU);
}

void GameLobby::Update(void)
{


	if (Network_Manager::GetInstance()->isConnected())
	{
		Network_Manager::GetInstance()->Dispatch(PacketQueueType::LOBBY);
	}
	
	Level_Manager::Get_Instance()->Update();

}

void GameLobby::Late_Update(void)
{
	Level_Manager::Get_Instance()->Late_Update();
	if (Level_Manager::Get_Instance()->GoToGamePlay())
		is_start = true;
}

void GameLobby::Render(void)
{
	HDC	hBackDC = Bmp_Manager::Get_Instance()->Find_Img(L"BackBuffer");
	Level_Manager::Get_Instance()->Render(hBackDC);
	BitBlt(m_hDC, 0, 0, LOBBY_WINCX, LOBBY_WINCY, hBackDC, 0, 0, SRCCOPY);
}

void GameLobby::Release(void)
{
    ReleaseDC(m_hWnd, m_hDC);
}

void GameLobby::InitBmps(HWND hand)
{
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/backbuffer.bmp", L"BackBuffer", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/Menu.bmp", L"Menu", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/muloff.bmp", L"multiOffButton", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/mulon.bmp", L"multiOnButton", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/sinoff.bmp", L"SingleOffButton", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/sinon.bmp", L"SingleOnButton", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/RoomListBack.bmp", L"RoomListBack", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/RoomList.bmp", L"RoomListbox", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/CreateOff.bmp", L"CreateOff", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/CreateOn.bmp", L"CreateOn", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/JoinOff.bmp", L"JoinOff", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/JoinOn.bmp", L"JoinOn", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/RefreshOff.bmp", L"RefreshOff", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/RefreshON.bmp", L"RefreshON", hand);
	
	
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/ExitOn.bmp", L"ExitOn", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/ExitOff.bmp", L"ExitOff", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/ReadyOff.bmp", L"ReadyOff", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/ReadyOn.bmp", L"ReadyOn", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/StartOn.bmp", L"StartOn", hand);
	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/StartOff.bmp", L"StartOff", hand);


	Bmp_Manager::Get_Instance()->Insert_Bmp(L"../bin/LobbyBmps/RoomBack.bmp", L"RoomBack", hand);


}


