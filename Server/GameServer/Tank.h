#pragma once
#include "GameObject.h"
class Tank : public GameObject
{

public:
	Tank();
	~Tank();

public:
		virtual void Initialize()	override;
		virtual int  Update()		override;
		virtual void Late_Update()	override;
		virtual void Release()		override;
public:					


	uint64 playerId = 0;	



};

