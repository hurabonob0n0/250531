#pragma once
#include "Level.h"

class Level_RoomList : public Level
{
public:
	Level_RoomList();
	virtual ~Level_RoomList();

public:

	virtual void Initialize() override;
	virtual int  Update() override;
	virtual void Late_Update() override;
	virtual void Render(HDC hDC) override;
	virtual void Release(void) override;

	void Show_RoomList();

private:

};

