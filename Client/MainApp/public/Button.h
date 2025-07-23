#pragma once
#include "GameObject.h"
class Button : public GameObject
{
public:
	Button();
	virtual ~Button();


public:
	virtual void Initialize() override;
	virtual int Update() override;
	virtual void Late_Update() override;
	virtual void Render(HDC hDC) override;
	virtual void Release(void) override;

public:
	void SetButtonID(BUTTON_ID id) { My_ID = id; };
	bool IsOnMouse() { return MouseOn; };
	bool GetButtonDown(){ return isClick; };

private:
	std::chrono::steady_clock::time_point _lastClickTime = std::chrono::steady_clock::now();
	BUTTON_ID My_ID;
	bool MouseOn;
	bool isClick;
};

