#include "Client_pch.h"
#include "RoomListBar.h"
#include "Bmp_Manager.h"

RoomListBar::RoomListBar()
{
}

RoomListBar::~RoomListBar()
{
}

void RoomListBar::Initialize()
{
	m_tInfo.fCX = 510.f;
	m_tInfo.fCY = 60.f;


}

int RoomListBar::Update()
{

    __super::Update_Rect();
	return 0;
}

void RoomListBar::Late_Update()
{
}

void RoomListBar::Render(HDC hDC)
{

    HDC	hMemDC = Bmp_Manager::Get_Instance()->Find_Img(L"RoomListbox");
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

    // 텍스트 영역 분할: 총 3등분
    const int totalWidth = (int)m_tInfo.fCX;
    const int sectionWidth = totalWidth / 3;

    int left = (int)(m_tInfo.fX - m_tInfo.fCX / 2.f);
    int top = (int)(m_tInfo.fY - m_tInfo.fCY / 2.f);
    int height = (int)m_tInfo.fCY;

    // 폰트 설정 (선택사항)
    HFONT hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("맑은 고딕"));
    HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(255, 255, 255));

    // RoomID 출력
    {
        RECT rc = { left +20, top, left + sectionWidth, top + height };
        wchar_t szRoomID[32] = {};
        swprintf_s(szRoomID, L"Room(%d)", Room_data.RoomID);
        DrawText(hDC, szRoomID, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    // Active/UnActive 출력
    {
        RECT rc = { left + sectionWidth + 20, top, left + sectionWidth * 2, top + height };
        const wchar_t* status = Room_data.isActive ? L"Active" : L"UnActive";
        DrawText(hDC, status, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    // (Cur/Max) 출력
    {
        RECT rc = { left + sectionWidth * 2 + 40, top, left + sectionWidth * 3, top + height };
        wchar_t szPlayer[32] = {};
        swprintf_s(szPlayer, L"(%d/%d)", Room_data.CurPlayer, Room_data.MaxPlayer);
        DrawText(hDC, szPlayer, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    // 폰트 원상복귀
    SelectObject(hDC, hOldFont);
    DeleteObject(hFont);
}

void RoomListBar::Release(void)
{
}

void RoomListBar::ShowRoomData()
{

}
