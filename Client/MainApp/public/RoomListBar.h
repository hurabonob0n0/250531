#pragma once
#include "GameObject.h"
class RoomListBar : public GameObject
{
public:
	RoomListBar();
	virtual ~RoomListBar();


public:
	virtual void Initialize() override;
	virtual int Update() override;
	virtual void Late_Update() override;
	virtual void Render(HDC hDC) override;
	virtual void Release(void) override;
public:
	bool IsOnMouse() { return MouseOn; };
	bool GetButtonDown() { return isClick; };

	void SetRoomData(Room_Data data) { Room_data = data; };
	void ShowRoomData();
private:
	bool MouseOn;
	bool isClick;
	Room_Data Room_data;
};

