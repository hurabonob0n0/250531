#include "Client_pch.h"
#include "Level_Room.h"
#include "Bmp_Manager.h"
#include "Object_Manager.h"
#include "Button.h"
#include "AbstractFactory.h"

Level_Room::Level_Room()
{
}

Level_Room::~Level_Room()
{
    Release();
}

void Level_Room::Initialize()
{
    Object_Manager::Get_Instance()->LevelChange = true;

    GameObject* StartButton = CAbstractFactory<Button>::Create_Button(860, 787, BUTTON_START);
    Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, StartButton);

    GameObject* ExitButton = CAbstractFactory<Button>::Create_Button(170, 787, BUTTON_EXIT);
    Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, ExitButton);

    GameObject* ReadyButton = CAbstractFactory<Button>::Create_Button(630, 787, BUTTON_READY);
    Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, ReadyButton);
}

int Level_Room::Update()
{
    if (Object_Manager::Get_Instance()->LevelChange)
        Object_Manager::Get_Instance()->LevelChange = false;


    Object_Manager::Get_Instance()->Update();
    return 0;
}

void Level_Room::Late_Update()
{

    Object_Manager::Get_Instance()->Late_Update();
}

void Level_Room::Render(HDC hDC)
{
    HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"RoomBack");
    BitBlt(hDC, 0, 0, LOBBY_WINCX, LOBBY_WINCY, hMemDC, 0, 0, SRCCOPY);
    Object_Manager::Get_Instance()->Render(hDC);

}

void Level_Room::Release(void)
{
}
