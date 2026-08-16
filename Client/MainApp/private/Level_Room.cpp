#include "Client_pch.h"
#include "Level_Room.h"
#include "Bmp_Manager.h"
#include "Object_Manager.h"
#include "Button.h"
#include "AbstractFactory.h"
#include "Room_Manager.h"
#include "Key_Manager.h"
#include "ClientPacketHandler.h"
#include "Network_Manager.h"



Level_Room::Level_Room()
    : lastClickTime(GetTickCount()) // 초기화
{
}

Level_Room::~Level_Room()
{
    Release();
}

void Level_Room::Initialize()
{
    Object_Manager::Get_Instance()->LevelChange = true;

    GameObject* StartButton = CAbstractFactory<Button>::Create_Button(860, 755, BUTTON_START);
    Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, StartButton);

    GameObject* ExitButton = CAbstractFactory<Button>::Create_Button(170, 755, BUTTON_EXIT);
    Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, ExitButton);

    GameObject* ReadyButton = CAbstractFactory<Button>::Create_Button(630, 755, BUTTON_READY);
    Object_Manager::Get_Instance()->Add_Object(OBJ_BUTTON, ReadyButton);

    for (int i = 0; i < 8; ++i)
    {
        int row = i / 2; // 0~3
        int col = i % 2; // 0 또는 1

        // 각 슬롯의 간격
        int slotGapX = 10;
        int slotGapY = 26;

        // 각 슬롯의 시작 위치
        int blueStartX = 80;
        int redStartX = 555;
        int startY = 130;

        int xOffset = col * (TEAM_SLOT_WIDTH + slotGapX);
        int yOffset = row * (TEAM_SLOT_HEIGHT + slotGapY);

        RECT blueRect = {
            blueStartX + xOffset,
            startY + yOffset,
            blueStartX + xOffset + TEAM_SLOT_WIDTH,
            startY + yOffset + TEAM_SLOT_HEIGHT
        };

        RECT redRect = {
            redStartX + xOffset,
            startY + yOffset,
            redStartX + xOffset + TEAM_SLOT_WIDTH,
            startY + yOffset + TEAM_SLOT_HEIGHT
        };

        BlueTeamRects.push_back(blueRect);
        RedTeamRects.push_back(redRect);

        Slot blueSlot = { blueRect, static_cast<uint8>(i + 1), false };
        Slot redSlot = { redRect, static_cast<uint8>(i + 9), false };

        Slots.push_back(blueSlot);
        Slots.push_back(redSlot);
    }
}
int Level_Room::Update()
{
    if (Object_Manager::Get_Instance()->LevelChange)
        Object_Manager::Get_Instance()->LevelChange = false;

    Object_Manager::Get_Instance()->Update();

    DWORD now = GetTickCount();

    if (Key_Manager::Get_Instance()->Key_Up(VK_LBUTTON) && now - lastClickTime >= 500)
    {
        lastClickTime = now;

        POINT pt;
        Lobby_GetCursorPos(g_hWnd, &pt);

        const auto& players = Room_Manager::Get_Instance()->GetRoomPlayerStates();

        // 슬롯 점유 상태 초기화
        for (auto& slot : Slots)
            slot.occupied = false;

        // 현재 점유된 슬롯 표시
        for (const auto& player : players)
        {
            for (auto& slot : Slots)
            {
                if (slot.position == player.Position)
                {
                    slot.occupied = true;
                    break;
                }
            }
        }

        // 클릭한 빈 슬롯 처리
        for (auto& slot : Slots)
        {
            if (::PtInRect(&slot.rect, pt) && !slot.occupied)
            {
                Room_Ready_Data myData;
                myData.PlayerID = Network_Manager::GetInstance()->GetMyClientID();
                myData.Position = slot.position;
                myData.Team = (slot.position <= 8); // true: Blue, false: Red
                myData.IsReady = false;

                Room_Manager::Get_Instance()->SetMyPlayerData(myData);
                auto sendBuffer = ClientPacketHandler::Make_C_CHANGE_INFO(myData);
                Network_Manager::GetInstance()->Send(sendBuffer);
                break;
            }
        }
    }

    return 0;
}

void Level_Room::Late_Update()
{
    Object_Manager::Get_Instance()->Late_Update();
}
void Level_Room::Render(HDC hDC)
{
    HDC hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"RoomBack");
    BitBlt(hDC, 0, 0, LOBBY_WINCX, LOBBY_WINCY, hMemDC, 0, 0, SRCCOPY);
    Object_Manager::Get_Instance()->Render(hDC);

    const auto& players = Room_Manager::Get_Instance()->GetRoomPlayerStates();

    // 1. 슬롯 점유 상태 초기화
    for (auto& slot : Slots)
        slot.occupied = false;

    // 2. 플레이어가 있는 슬롯 점유 표시
    for (const auto& player : players)
    {
        for (auto& slot : Slots)
        {
            if (slot.position == player.Position)
            {
                slot.occupied = true;
                break;
            }
        }
    }

    // 3. 슬롯 테두리와 텍스트 출력
    for (const auto& slot : Slots)
    {
        // 디버깅용임
        //Rectangle(hDC, slot.rect.left, slot.rect.top, slot.rect.right, slot.rect.bottom);

        for (const auto& player : players)
        {
            if (slot.position == player.Position)
            {
                std::wstring text = L"ID: " + std::to_wstring(player.PlayerID);
                if (player.IsReady)
                    text += L" (Ready)";

                // --------------------
                // 폰트 설정 (맑은 고딕, 크기 20, Bold)
                HFONT hFont = CreateFont(30, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Impact");

                HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

                // 배경 투명, 글자 검정색
                SetBkMode(hDC, TRANSPARENT);
                SetTextColor(hDC, RGB(200, 200, 200));

                // 텍스트 출력
                DrawText(hDC, text.c_str(), -1, const_cast<RECT*>(&slot.rect), DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                // 폰트 복구 및 제거
                SelectObject(hDC, hOldFont);
                DeleteObject(hFont);
                // --------------------

                break;
            }
        }
    }
}
void Level_Room::Release()
{
    // 슬롯 정보 해제
    Slots.clear();

    // 팀 슬롯 RECT 정리 (필요 없으면 이거 생략 가능)
    BlueTeamRects.clear();
    RedTeamRects.clear();

    // 오브젝트도 전부 제거 (필요 시)
    Object_Manager::Get_Instance()->DeleteID(OBJ_BUTTON);
}