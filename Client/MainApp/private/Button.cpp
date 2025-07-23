#include "Client_pch.h"
#include "Client_Globals.h"
#include "Button.h"
#include "Key_Manager.h"
#include "Bmp_Manager.h"
#include "Level_Manager.h"
#include "Object_Manager.h"
#include "Network_Manager.h"
#include  "ClientPacketHandler.h"

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
	default:
		break;

	}

}

int Button::Update()
{
	POINT	pt{};
	GetCursorPos(&pt);
	ScreenToClient(g_hWnd, &pt);

	if (PtInRect(&m_tRect, pt))
	{
		if (Key_Manager::Get_Instance()->Key_Down(VK_LBUTTON))
		{

			isClick = true;

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
			break;

		case BUTTON_MULTIPLAY:
			m_bDead = true;
			Level_Manager::Get_Instance()->Level_Change(LEVEL_ROOMLIST);
		
			MultiPlay = true;
			break;
		case BUTTON_CREATE: {
			auto sendBuffer = ClientPacketHandler::Make_C_CREATEROOM(1);
			Network_Manager::GetInstance()->Send(sendBuffer);
		}
			break;
		case BUTTON_JOIN: {

			auto sendBuffer = ClientPacketHandler::Make_C_JOINROOM(1);
			Network_Manager::GetInstance()->Send(sendBuffer);

		}

			break;
		case BUTTON_REFRESH:
		{
			auto sendBuffer = ClientPacketHandler::Make_C_SHOWROOM(1);
			Network_Manager::GetInstance()->Send(sendBuffer);
			isClick = false;
		}
			break;
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
	default:
		break;

	}
}

void Button::Release(void)
{
}
