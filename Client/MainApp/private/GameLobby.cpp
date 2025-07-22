#include "Client_pch.h"
#include "GameLobby.h"

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

}

void GameLobby::Update(void)
{
    ++Timer;


}

void GameLobby::Late_Update(void)
{
    if (Timer > 600) {
        is_start = true;
    }
}

void GameLobby::Render(void)
{
    RECT rect;
    GetClientRect(m_hWnd, &rect);

    HBRUSH hBrush = CreateSolidBrush(RGB(0, 120, 255)); // ¹àÀº ÆÄ¶û
    FillRect(m_hDC, &rect, hBrush);
    DeleteObject(hBrush);
}

void GameLobby::Release(void)
{
    ReleaseDC(m_hWnd, m_hDC);
}
