#pragma once
#include "Level.h"

struct Slot {
	RECT rect;
	int position;
	bool occupied;
};

class Level_Room : public Level
{
public:
	Level_Room();
	virtual ~Level_Room();

public:

	virtual void Initialize() override;
	virtual int  Update() override;
	virtual void Late_Update() override;
	virtual void Render(HDC hDC) override;
	virtual void Release(void) override;
public:

private:
	const int TEAM_SLOT_WIDTH = 190;
	const int TEAM_SLOT_HEIGHT = 115;

	std::vector<RECT> BlueTeamRects;
	std::vector<RECT> RedTeamRects;
	std::vector<Slot> Slots;
	DWORD lastClickTime;
};

