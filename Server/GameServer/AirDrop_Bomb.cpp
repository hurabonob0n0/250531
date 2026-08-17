#include "pch.h"
#include "AirDrop_Bomb.h"

AirDrop_Bomb::AirDrop_Bomb()
{
}

AirDrop_Bomb::~AirDrop_Bomb()
{
}

void AirDrop_Bomb::Initialize()
{
}

int AirDrop_Bomb::Update(float deltaTime)
{

	if (_isDead) {
		return 1;
	}

	ProcessMove(deltaTime);

	return 0;
}

void AirDrop_Bomb::Late_Update()
{
}

void AirDrop_Bomb::Release()
{
}

void AirDrop_Bomb::SetInitData(uint8 playerID, uint8 TankIndex ,Vec3 InitPos)
{
	OwnerID = playerID;
	_myPos = InitPos;
	_prevPos = InitPos;     // 선분 판정의 시작점 (Weapon::_prevPos 주석 참고)
	OwnerTankIndex = TankIndex;
}

void AirDrop_Bomb::ProcessMove(float deltaTime)
{
	Save_PrevPos();     // 이번 틱의 선분 시작점

	_myPos.Y -= speed * deltaTime;
}
