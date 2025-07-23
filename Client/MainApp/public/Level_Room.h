#pragma once
#include "Level.h"

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

private:

};

