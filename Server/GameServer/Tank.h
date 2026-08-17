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
	void SetUnSpawn(int64 nowMs) { Spawn = false; _deadAtMs = nowMs; };

public:
	// ================================================================
	//  서버 검증용 상태 (안티치트)
	// ================================================================

	int64 GetLastShotMs() const { return _lastShotMs; }
	void  SetLastShotTime(int64 nowMs) { _lastShotMs = nowMs; }

	int64 GetDeadTimeMs()   const { return _deadAtMs; }

	/*  정당한 순간이동(스폰/리스폰) 뒤에 부른다.
	    안 부르면 스폰 좌표로 튄 것이 속도 위반으로 잡힌다.  */
	void  ResetMoveCheck(int64 nowMs, const Vec3& pos)
	{
		_lastMoveMs      = nowMs;
		_allowedMoveDist      = MAX_ALLOWED_MOVE_DIST;
		_lastAcceptedPos = pos;
	}

	/*  이동 예산을 갱신하고 이번 이동이 허용되는지 답한다.
	    허용되면 예산에서 이동 거리를 뺀다(거부되면 빼지 않는다).  */
	bool CheckMoveSpeed(const Vec3& newPos, int64 nowMs);

	uint64				playerId = 0;
private:
	float				_posinAngle = 0.f;
	float				_potapAngle = 0.f;

	/*   서버 로직은 이 둘을 읽지 않고 받은 걸 그대로 들고 있다가 그대로 내보낸다    */
	Quat				_rot;
	int8				_wheelSag[TANK_WHEEL_COUNT] = {};
	
	
	bool Spawn;

	// ---- 서버 검증 상태 ----
	int64				_lastShotMs = 0;    // 마지막 발포 시각(서버 기준)
	int64				_deadAtMs   = 0;    // 사망 시각. 리스폰 쿨타임의 기준
	int64				_lastMoveMs = 0;    // 마지막 위치 갱신 시각
	float				_allowedMoveDist = 0.f;  // 남은 이동 허용량(유닛)
	Vec3				_lastAcceptedPos;   // 마지막으로 통과시킨 좌표

	OBB					_obbBox;
	std::vector<Room_Ready_Data> passengers;
	TankSize MySize = { 3.8f, 3.7f, 9.6f };

};

