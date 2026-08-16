#pragma once
#include "TransformObject.h"


/*-------------------

	Tank_Info


	//폭		380
	//길이	960
	//높이	370


-------------------*/


struct TankSize {

	float Width;
	float Hight;
	float Length;

};


class Tank : public TransformObject
{

public:
	Tank();
	~Tank();

public:
		virtual void Initialize()					override;
		virtual int  Update(float deltaTime)		override;
		virtual void Late_Update()					override;
		virtual void Release()						override;
public:					

	void SetTankState(const Matrix4x4& mat,float PosinAngle, float PotapAngle);
	void SetTankOnlyPos(const Matrix4x4& mat);
	void SetTankOnlyPosin(float PosinAngle, float PotapAngle);

	/*  클라가 보내오는 형태(위치 + 쿼터니언 + 바퀴)를 그대로 받는 경로.
     */
	void SetTankStateQuat(const Vec3& pos, const Quat& rot,
						  float PosinAngle, float PotapAngle,
						  const int8* pWheelSag);
	void SetTankOnlyPosQuat(const Vec3& pos, const Quat& rot, const int8* pWheelSag);

	Tank_INFO GetTankState();
	void UpdateOBBFromTransform();
	OBB2D GetOBB2D() const;
	void Damage(int dmg);
	bool IsDead();
	bool HasPassenger(uint8 playerID) const;
	const OBB& Get_OBB() const { return _obbBox; }



	void AddPassenger(const Room_Ready_Data& data)
	{
		passengers.push_back(data);
	}

	const std::vector<Room_Ready_Data>& GetPassengers() const
	{
		return passengers;
	}

	void SetSpawn(const Matrix4x4& mat, float PosinAngle, float PotapAngle);
	bool isSpawned() { return Spawn; };
	void SetUnSpawn() { Spawn = false; };

	uint64				playerId = 0;
private:
	float				_posinAngle = 0.f;
	float				_potapAngle = 0.f;

	/*   서버 로직은 이 둘을 읽지 않고 받은 걸 그대로 들고 있다가 그대로 내보낸다    */
	Quat				_rot;
	int8				_wheelSag[TANK_WHEEL_COUNT] = {};
	
	
	bool Spawn;

	OBB					_obbBox;
	std::vector<Room_Ready_Data> passengers;
	TankSize MySize = { 3.8f, 3.7f, 9.6f };

};

