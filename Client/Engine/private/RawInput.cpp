#include "RawInput.h"

IMPLEMENT_SINGLETON(CRawInput)

inline CRawInput::CRawInput() {}

HRESULT CRawInput::Initialize(HWND hWnd)
{
	RAWINPUTDEVICE Rid[2] = {};

	// 키보드
	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x06;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = hWnd;

	// 마우스
	Rid[1].usUsagePage = 0x01;
	Rid[1].usUsage = 0x02;
	Rid[1].dwFlags = RIDEV_INPUTSINK;
	Rid[1].hwndTarget = hWnd;

	if (!RegisterRawInputDevices(Rid, 2, sizeof(RAWINPUTDEVICE)))
		return E_FAIL;

	return S_OK;
}

void CRawInput::Update_InputDev(LPARAM lParam)
{
	UINT dwSize = 0;
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
	std::vector<BYTE> lpb(dwSize);
	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.data(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
		return;

	RAWINPUT* raw = (RAWINPUT*)lpb.data();

	if (raw->header.dwType == RIM_TYPEKEYBOARD)
	{
		UINT vkey = raw->data.keyboard.VKey;
		bool isDown = !(raw->data.keyboard.Flags & RI_KEY_BREAK);
		if (isDown)
		{
			m_KeyState[vkey] |= KEY_DOWN;   // KeyDown만 ON
			m_KeyState[vkey] &= ~KEY_UP;    // KeyUp OFF
		}
		else
		{
			m_KeyState[vkey] |= KEY_UP;     // KeyUp ON
			m_KeyState[vkey] &= ~KEY_DOWN;  // KeyDown OFF
		}
	}
	else if (raw->header.dwType == RIM_TYPEMOUSE)
	{
		RAWMOUSE& mouse = raw->data.mouse;

		// [수정 전] m_MouseDelta = lLastX (덮어쓰기)
		// WM_INPUT 은 마우스가 움직일 때마다(보통 초당 125~1000번) 하나씩 온다. 한 프레임 사이에 여러 개가 오면
		// 마지막 하나만 남고 나머지 이동량은 버려졌다 → fps 가 낮을수록(프레임이 길수록) 버리는 양이 많아 감도가 낮아졌다
		// 프레임 동안 받은 이동량을 전부 더하고, ResetPerFrame() 에서 0 으로 되돌린다 → fps 와 상관없이 같은 손 움직임 = 같은 회전
		// (MOUSE_MOVE_ABSOLUTE 는 원격 데스크톱/태블릿이 보내는 절대 좌표라 이동량이 아니다 → 더하지 않는다)
		if (!(mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
		{
			m_MouseDelta[0] += static_cast<SHORT>(mouse.lLastX);
			m_MouseDelta[1] += static_cast<SHORT>(mouse.lLastY);
		}

		if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
			m_bMouseCurrState[0] = true;
		if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
			m_bMouseCurrState[0] = false;

		if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
			m_bMouseCurrState[1] = true;
		if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
			m_bMouseCurrState[1] = false;

		if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
			m_bMouseCurrState[2] = true;
		if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
			m_bMouseCurrState[2] = false;

		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
			m_bMouseCurrState[3] = true;
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
			m_bMouseCurrState[3] = false;

		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
			m_bMouseCurrState[4] = true;
		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
			m_bMouseCurrState[4] = false;

		// 휠도 같은 문제: 예전에는 휠 메시지 뒤에 이동 메시지가 하나만 와도 0 으로 지워졌다. 누적하고 프레임마다 초기화
		if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
			m_MouseDelta[2] += static_cast<SHORT>(mouse.usButtonData);
	}
}

bool CRawInput::Key_Down(UINT vkey)
{
	return !m_bKeyPrevState[vkey] && (m_KeyState[vkey] & KEY_DOWN);
}

bool CRawInput::Key_Up(UINT vkey)
{
	return m_bKeyPrevState[vkey] && (m_KeyState[vkey] & KEY_UP);
}

bool CRawInput::Key_Pressing(UINT vkey)
{
	return m_bKeyPrevState[vkey] || (m_KeyState[vkey] & KEY_DOWN);
}

bool CRawInput::Mouse_Down(UINT button)
{
	/*if (!m_bMousePrevState[button] && m_bMouseCurrState[button])
	{
		m_bMousePrevState[button] = true;
		return true;
	}
	if (m_bMousePrevState[button] && !m_bMouseCurrState[button])
		m_bMousePrevState[button] = false;

	return false;*/

	return m_bMouseCurrState[button] && !m_bMousePrevState[button];
}

bool CRawInput::Mouse_Up(UINT button)
{
	/*if (m_bMousePrevState[button] && !m_bMouseCurrState[button])
	{
		m_bMousePrevState[button] = false;
		return true;
	}
	if (!m_bMousePrevState[button] && m_bMouseCurrState[button])
		m_bMousePrevState[button] = true;

	return false;*/

	return !m_bMouseCurrState[button] && m_bMousePrevState[button];
}

bool CRawInput::Mouse_Pressing(UINT button)
{
	return m_bMouseCurrState[button] && m_bMousePrevState[button];
}

void CRawInput::ResetPerFrame()
{
	//// Mouse delta 초기화
	//m_MouseDelta[0] = 0;
	//m_MouseDelta[1] = 0;
	//m_MouseDelta[2] = 0;

	memcpy(m_bMousePrevState, m_bMouseCurrState, sizeof(m_bMousePrevState));
	ZeroMemory(m_MouseDelta, sizeof(m_MouseDelta));
}

void CRawInput::UpdateKeyStates()
{
	for (int i = 0; i < 256; ++i)
	{
		m_bKeyPrevState[i] = (m_KeyState[i] & KEY_DOWN || m_KeyState[i] & KEY_PRESSING);
	}
}

void CRawInput::Free()
{
}
