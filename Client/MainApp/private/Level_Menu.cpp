#include "Client_pch.h"
#include "Level_Menu.h"
#include "AbstractFactory.h"
#include "Bmp_Manager.h"
#include "Object_Manager.h"
#include "Button.h"

Level_Menu::Level_Menu()
{
}

Level_Menu::~Level_Menu()
{
	Release();
}

void Level_Menu::Initialize()
{
	GameObject* SinglePlay_Button = CAbstractFactory<Button>::Create_Button(304,750,BUTTON_SINGLEPLAY);
	Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, SinglePlay_Button);

	GameObject* MultiPlay_Button = CAbstractFactory<Button>::Create_Button(720, 750, BUTTON_MULTIPLAY);
	Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, MultiPlay_Button);
}

int Level_Menu::Update()
{

	Object_Manager::Get_Instance()->Update();
	return 0;
}

void Level_Menu::Late_Update()
{
	Object_Manager::Get_Instance()->Late_Update();
}

void Level_Menu::Render(HDC hDC)
{
	HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"Menu");
	BitBlt(hDC, 0, 0, LOBBY_WINCX, LOBBY_WINCY, hMemDC, 0, 0, SRCCOPY);
	Object_Manager::Get_Instance()->Render(hDC);
}

void Level_Menu::Release(void)
{
	Object_Manager::Get_Instance()->DeleteID(OBJ_BUTTON);
}
