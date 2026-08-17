#pragma once
#include "GameObject.h"
class Weapon : public GameObject
{

public:
	Weapon();
	~Weapon();

public:
	virtual void Initialize()					PURE;
	virtual int  Update(float deltaTime)		PURE;
	virtual void Late_Update()					PURE;
	virtual void Release()						PURE;

protected:
	


	float Move_Speed;
	float InitPos;


	WEAPON_ID	myWeaponID;
	uint8		OwnerID;
	uint8		OwnerTankIndex;
	Vec3		hitPos;

	/*  직전 틱의 위치. 충돌 판정을 점이 아니라 "직전 -> 현재" 선분으로 하기 위한 것.
	    ★ 반드시 SetInitData 에서 시작 좌표로 초기화할 것.
	      기본값(0,0,0) 인 채로 첫 틱을 맞으면 원점에서 총구까지 맵을 가로지르는
	      선분이 만들어져, 그 경로에 있는 모든 것에 발사하자마자 맞는다.        */
	Vec3		_prevPos;

public:
	const Vec3& GetPrevPos() const { return _prevPos; }

protected:
	// 이동 직전에 부른다.
	void Save_PrevPos() { _prevPos = _myPos; }


};

