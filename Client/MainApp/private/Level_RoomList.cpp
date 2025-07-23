#include "Client_pch.h"
#include "Level_RoomList.h"
#include "Network_Manager.h"
#include "AbstractFactory.h"
#include "Object_Manager.h"
#include "Bmp_Manager.h"

Level_RoomList::Level_RoomList()
{
}

Level_RoomList::~Level_RoomList()
{
	Release();
}

void Level_RoomList::Initialize()
{
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
}

void Level_RoomList::Release(void)
{

}

void Level_RoomList::Show_RoomList()
{
}
