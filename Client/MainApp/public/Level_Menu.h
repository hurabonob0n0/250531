#pragma once
#include "Level.h"

class Level_Menu : public Level
{
public:
	Level_Menu();
	virtual ~Level_Menu();

public:

	virtual void Initialize() override;
	virtual int  Update() override;
	virtual void Late_Update() override;
	virtual void Render(HDC hDC) override;
	virtual void Release(void) override;

};

