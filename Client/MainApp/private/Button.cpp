#include "Client_pch.h"
#include "Client_Globals.h"
#include "Button.h"
#include "Key_Manager.h"
#include "Bmp_Manager.h"
#include "Level_Manager.h"
#include "Object_Manager.h"
#include "Network_Manager.h"
#include "ClientPacketHandler.h"
#include "Room_Manager.h"

Button::Button()
{
}

Button::~Button()
{
}

void Button::Initialize()
{
	MouseOn = false;
	isClick = false;
	switch (My_ID) {
	case BUTTON_SINGLEPLAY:
		m_tInfo.fCX = 268.f;
		m_tInfo.fCY = 116.f;
		break;

	case BUTTON_MULTIPLAY:
		m_tInfo.fCX = 268.f;
		m_tInfo.fCY = 116.f;
		break;
	case BUTTON_CREATE:
		m_tInfo.fCX = 268.f;
		m_tInfo.fCY = 125.f;
		break;
	case BUTTON_JOIN:
		m_tInfo.fCX = 268.f;
		m_tInfo.fCY = 125.f;
		break;
	case BUTTON_REFRESH:
		m_tInfo.fCX = 268.f;
		m_tInfo.fCY = 125.f;
		break;

	case BUTTON_EXIT:
		m_tInfo.fCX = 223.f;
		m_tInfo.fCY = 105.f;
		break;

	case BUTTON_READY:
		m_tInfo.fCX = 223.f;
		m_tInfo.fCY = 105.f;
		break;

	case BUTTON_START:
		m_tInfo.fCX = 223.f;
		m_tInfo.fCY = 105.f;
		break;
	default:
		break;

	}

}

int Button::Update()
{
	POINT	pt{};
	Lobby_GetCursorPos(g_hWnd, &pt);

	if (PtInRect(&m_tRect, pt))
	{
		if (Key_Manager::Get_Instance()->Key_Down(VK_LBUTTON))
		{

			auto now = std::chrono::steady_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastClickTime);

			if (duration.count() >= 1000) 
			{
				isClick = true;
				_lastClickTime = now;
			}

		}
		MouseOn = true;
	}
	else
		MouseOn = false;


	if (isClick) {

		switch (My_ID) {
		case BUTTON_SINGLEPLAY:
			m_bDead = true;
			MultiPlay = false;
			Level_Manager::Get_Instance()->SetGamePlayMode();
			isClick = false;
			break;

		case BUTTON_MULTIPLAY:
			m_bDead = true;
			Level_Manager::Get_Instance()->Level_Change(LEVEL_ROOMLIST);
		
			MultiPlay = true;
			isClick = false;
			break;
		case BUTTON_CREATE: {
			auto sendBuffer = ClientPacketHandler::Make_C_CREATEROOM(1);
			Network_Manager::GetInstance()->Send(sendBuffer);
			isClick = false;
		}
			break;
		case BUTTON_JOIN: {
			
			auto sendBuffer = ClientPacketHandler::Make_C_JOINROOM(Room_Manager::Get_Instance()->GetChoiceRoom());
			Network_Manager::GetInstance()->Send(sendBuffer);
			isClick = false;

		}

			break;
		case BUTTON_REFRESH:
		{
			auto sendBuffer = ClientPacketHandler::Make_C_SHOWROOM(1);
			Network_Manager::GetInstance()->Send(sendBuffer);
			isClick = false;
		}
			break;

		case BUTTON_READY: {
			auto sendBuffer = ClientPacketHandler::Make_C_READY(1);
			Network_Manager::GetInstance()->Send(sendBuffer);
			isClick = false;

		}
						 break;
		case BUTTON_START: {

			auto sendBuffer = ClientPacketHandler::Make_C_START(1);
			Network_Manager::GetInstance()->Send(sendBuffer);
			isClick = false;

		}
						 break;
		case BUTTON_EXIT:{
			auto sendBuffer = ClientPacketHandler::Make_C_EXITROOM(Room_Manager::Get_Instance()->GetChoiceRoom());
			Network_Manager::GetInstance()->Send(sendBuffer);
			Level_Manager::Get_Instance()->Level_Change(LEVEL_ROOMLIST);
			Room_Manager::Get_Instance()->ChoiceRoom(ROBBY);
			isClick = false;
		}
		default:
			break;


		}
	}

	if (m_bDead)
		return OBJ_DEAD;

	__super::Update_Rect();
	return OBJ_NOEVENT;
}

void Button::Late_Update()
{



	
}

void Button::Render(HDC hDC)
{
	switch (My_ID) {
	case BUTTON_SINGLEPLAY:
		if (MouseOn) {

			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"SingleOnButton");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		else {
			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"SingleOffButton");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}

		break;

	case BUTTON_MULTIPLAY:
		if (MouseOn) {

			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"multiOnButton");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		else {
			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"multiOffButton");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		break;
	case BUTTON_CREATE:
		if (MouseOn) {

			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"CreateOn");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		else {
			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"CreateOff");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		break;
	case BUTTON_JOIN:
		if (MouseOn) {

			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"JoinOn");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		else {
			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"JoinOff");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		break;
	case BUTTON_REFRESH:
		if (MouseOn) {

			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"RefreshON");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		else {
			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"RefreshOff");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		break;
	case BUTTON_EXIT:
		if (MouseOn) {

			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"ExitOn");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		else {
			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"ExitOff");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		break;

	case BUTTON_READY:
		if (MouseOn) {

			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"ReadyOn");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		else {
			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"ReadyOff");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		break;


	case BUTTON_START:
		if (MouseOn) {

			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"StartOn");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
		else {
			HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"StartOff");
			GdiTransparentBlt(
				hDC,
				m_tRect.left,
				m_tRect.top,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				hMemDC,
				0,
				0,
				(int)m_tInfo.fCX,
				(int)m_tInfo.fCY,
				RGB(255, 255, 255));
		}
	default:
		break;

	}
}

void Button::Release(void)
{
}
