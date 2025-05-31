#pragma once
#include "Define.h"

class GameObject
{

public:

	GameObject();
	~GameObject();

public:
	virtual void Initialize()	PURE;
	virtual int	 Update()		PURE;
	virtual void Late_Update()	PURE;
	virtual void Release()		PURE;

public:

	void SetPos(float a, float b, float c) { Pos.PosX = a, Pos.PosY = b, Pos.PosZ = c; };
	Position GetPos() { return Pos; };

protected:

	Position Pos{0,0,0};

};

