#pragma once
#include "ObjectManager.h"


struct Vec3_Data;

class Room 
{
public:
	Room();
	Room(uint64 Max);
	~Room();

public:
	void Initialize();
	void Update();
	void LateUpdate();
	void Release();

public:
	void Accept_Player(PlayerRef Player);
	void Leave_Player(PlayerRef Player);
	void Start();
	void Broadcast(SendBufferRef sendBuffer);

	void ShowPos();

	bool Check_Full(uint16 MaxPlayer);
	



public:

	void MovePlayer(int64 playerID, float x, float y, float z);
	void SetPlayerPos(int64 pID, float x, float y, float z);
	Vec3_Data GetPlayerPos(int64 pID);


private:
	
	USE_LOCK;
	map<uint64, PlayerRef>	_Players;

public:
	bool isStart = false;

private:
	float x1;
	float y1;
	float z1;
	float x2;
	float y2;
	float z2;

};

extern Room GRoom;