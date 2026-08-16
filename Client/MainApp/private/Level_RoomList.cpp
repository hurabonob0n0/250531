#include "Client_pch.h"
#include "Level_RoomList.h"
#include "Network_Manager.h"
#include "AbstractFactory.h"
#include "Object_Manager.h"
#include "Bmp_Manager.h"
#include "Room_Manager.h"
#include "RoomListBar.h"
#include "Key_Manager.h"

Level_RoomList::Level_RoomList()
{
}

Level_RoomList::~Level_RoomList()
{
	Release();
}

void Level_RoomList::Initialize()
{
	if(!Network_Manager::GetInstance()->isConnected())

	Network_Manager::GetInstance()->Initialize(L"127.0.0.1", 7777);

	Object_Manager::Get_Instance()->LevelChange = true;

	GameObject* Create_Button = CAbstractFactory<Button>::Create_Button(800, 400, BUTTON_CREATE);
	Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, Create_Button);

	GameObject* Join_Button = CAbstractFactory<Button>::Create_Button(800, 550, BUTTON_JOIN);
	Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, Join_Button);

	GameObject* Refresh_Button = CAbstractFactory<Button>::Create_Button(800, 700, BUTTON_REFRESH);
	Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, Refresh_Button);
}

int Level_RoomList::Update()
{



	if (Object_Manager::Get_Instance()->LevelChange)
		Object_Manager::Get_Instance()->LevelChange = false;


	if (Room_Manager::Get_Instance()->isChange) {
		CurRoomData = nullptr;
		Set_RoomList();
		Room_Manager::Get_Instance()->isChange = false;
	}


	POINT pt{};
	Lobby_GetCursorPos(g_hWnd, &pt);

	for (auto& pair : m_vecRoomRects)
	{
		RECT rc = pair.first;
		if (PtInRect(&rc, pt))
		{
			if (Key_Manager::Get_Instance()->Key_Down(VK_LBUTTON))
			{
				CurRoomData = &pair.second;
				Room_Manager::Get_Instance()->ChoiceRoom(CurRoomData->RoomID);
				
			}
		}
	}


	Object_Manager::Get_Instance()->Update();
	return 0;
}

void Level_RoomList::Late_Update()
{
	Object_Manager::Get_Instance()->Late_Update();
}

void Level_RoomList::Render(HDC hDC)
{
	HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"RoomListBack");
	BitBlt(hDC, 0, 0, LOBBY_WINCX, LOBBY_WINCY, hMemDC, 0, 0, SRCCOPY);
	Object_Manager::Get_Instance()->Render(hDC);

	if (CurRoomData != nullptr)
	{
		// 출력 위치 (오른쪽 상단 ROOM INFO 박스 안)
		int infoX = 700;
		int infoY = 175;

		wchar_t szBuffer[128] = {};

		swprintf_s(szBuffer, L"방 번호 : %d", CurRoomData->RoomID);
		TextOut(hDC, infoX, infoY, szBuffer, wcslen(szBuffer));

		swprintf_s(szBuffer, L"인원 : %d / %d", CurRoomData->CurPlayer, CurRoomData->MaxPlayer);
		TextOut(hDC, infoX, infoY + 30, szBuffer, wcslen(szBuffer));

	}
}

void Level_RoomList::Release(void)
{

	Object_Manager::Get_Instance()->DeleteID(OBJ_BUTTON);
	Object_Manager::Get_Instance()->DeleteID(OBJ_ROOMLISTBAR);
	m_vecRoomRects.clear();
	m_vecRoomBars.clear();
	CurRoomData = nullptr;

	Room_Manager::Get_Instance()->ClearRoomList();

}

void Level_RoomList::Show_RoomList()
{
}

void Level_RoomList::Set_RoomList()
{
	//Object_Manager::Get_Instance()->DeleteID(OBJ_ROOMLISTBAR);

	//const float startX = 320.f;
	//const float startY = 190.f;
	//const float gapY = 63.f;

	//m_vecRoomBars = Room_Manager::Get_Instance()->GetRoomList();

	//for (size_t i = 0; i < m_vecRoomBars.size(); ++i)
	//{
	//	// RoomListBar 객체 생성
	//	GameObject* pBar = CAbstractFactory<RoomListBar>::Create(startX, startY + i * gapY);
	//	dynamic_cast<RoomListBar*>(pBar)->SetRoomData(m_vecRoomBars[i]);
	//	Object_Manager::Get_Instance()->Add_Object(OBJ_ROOMLISTBAR, pBar);
	//}

	Object_Manager::Get_Instance()->DeleteID(OBJ_ROOMLISTBAR);
	m_vecRoomBars = Room_Manager::Get_Instance()->GetRoomList();

	const float startX = 320.f;
	const float startY = 190.f;
	const float gapY = 63.f;

	m_vecRoomRects.clear(); // 클릭 영역 초기화

	for (size_t i = 0, visualIdx = 0; i < m_vecRoomBars.size(); ++i)
	{
		if (m_vecRoomBars[i].isActive == false)
			continue;

		GameObject* pBar = CAbstractFactory<RoomListBar>::Create(startX, startY + visualIdx * gapY);
		dynamic_cast<RoomListBar*>(pBar)->SetRoomData(m_vecRoomBars[i]);
		Object_Manager::Get_Instance()->Add_Object(OBJ_ROOMLISTBAR, pBar);

		// 클릭 영역 저장
		RECT rc = {
			static_cast<LONG>(startX - 105),                // x - width/2
			static_cast<LONG>(startY + visualIdx * gapY - 30), // y - height/2
			static_cast<LONG>(startX + 105),
			static_cast<LONG>(startY + visualIdx * gapY + 30)
		};
		m_vecRoomRects.push_back(std::make_pair(rc, m_vecRoomBars[i]));
		++visualIdx;
	}

}
