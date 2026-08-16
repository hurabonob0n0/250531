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

	/*  예전엔 BitBlt 로 1:1 복사였다. 창이 1024x880 보다 작으면(세로 768 노트북)
		아래쪽 버튼이 그대로 잘려서 보이지 않았다.
		이제 창 크기에 맞춰 늘린다. 마우스는 Lobby_GetCursorPos 가 반대로 되돌리므로
		클릭 위치는 그대로 맞는다 - 둘은 항상 같이 고쳐야 한다.                     */
	RECT rcClient = {};
	GetClientRect(m_hWnd, &rcClient);

	if (rcClient.right == LOBBY_WINCX && rcClient.bottom == LOBBY_WINCY)
	{
		BitBlt(m_hDC, 0, 0, LOBBY_WINCX, LOBBY_WINCY, hBackDC, 0, 0, SRCCOPY);
	}
	else
	{
		/* HALFTONE 이라야 축소할 때 글자가 뭉개지지 않는다.
		   이 모드는 SetBrushOrgEx 를 같이 불러줘야 한다(GDI 규약). */
		SetStretchBltMode(m_hDC, HALFTONE);
		SetBrushOrgEx(m_hDC, 0, 0, nullptr);

		StretchBlt(m_hDC, 0, 0, rcClient.right, rcClient.bottom,
				   hBackDC, 0, 0, LOBBY_WINCX, LOBBY_WINCY, SRCCOPY);
	}
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


