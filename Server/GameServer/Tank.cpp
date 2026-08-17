#include "pch.h"
#include "Tank.h"

Tank::Tank()
{
	Spawn = true;
	_hp = 100;
}

Tank::~Tank()
{
}

void Tank::Initialize()
{

}

int Tank::Update(float deltaTime)
{
	SyncPosFromMatrix();
	UpdateOBBFromTransform();
	return 0;
}

void Tank::Late_Update()
{
}

void Tank::Release()
{

}

void Tank::SetTankState(const Matrix4x4& mat, float PotapAngle, float PosinAngle)
{
	__super::SetTransform(mat);
	_potapAngle = PotapAngle;
	_posinAngle = PosinAngle;

}

void Tank::SetTankOnlyPos(const Matrix4x4& mat)
{
	__super::SetTransform(mat);
}

void Tank::SetTankOnlyPosin(float PosinAngle, float PotapAngle)
{
	_potapAngle = PotapAngle;
	_posinAngle = PosinAngle;
}

void Tank::SetTankStateQuat(const Vec3& pos, const Quat& rot,
							float PosinAngle, float PotapAngle,
							const int8* pWheelSag)
{
	SetTankOnlyPosQuat(pos, rot, pWheelSag);

	_potapAngle = PotapAngle;
	_posinAngle = PosinAngle;
}

void Tank::SetTankOnlyPosQuat(const Vec3& pos, const Quat& rot, const int8* pWheelSag)
{
	_rot = rot;

	/* 서버 로직(GetPos)과 OBB 는 행렬을 읽으므로 여기서 한 번 펴 둔다. */
	__super::SetTransform(Matrix4x4::CreateFromQuatPos(rot, pos.X, pos.Y, pos.Z));

	if (pWheelSag)
		std::memcpy(_wheelSag, pWheelSag, sizeof(_wheelSag));
}

Tank_INFO Tank::GetTankState()
{
	Tank_INFO myInfo{};

	/*  위치는 행렬에서 뽑는다. 스폰처럼 행렬로 들어오는 경로가 아직 있어서,
		받은 값을 그대로 되돌리면 그쪽이 빠진다.                              */
	myInfo.Pos = Vec3(TransformMatrix.m[3][0],
					  TransformMatrix.m[3][1],
					  TransformMatrix.m[3][2]);
	myInfo.Rot        = _rot;
	myInfo.PosinAngle = _posinAngle;
	myInfo.PotapAngle = _potapAngle;
	myInfo.TankHP     = _hp;

	std::memcpy(myInfo.WheelSag, _wheelSag, sizeof(_wheelSag));

	return myInfo;
}

void Tank::UpdateOBBFromTransform()
{
	Matrix4x4& mat = TransformMatrix;

	_obbBox.center = Vec3(
		mat.m[3][0],
		mat.m[3][1] + MySize.Hight * 0.5f,
		mat.m[3][2]
	);

	_obbBox.axis[0] = Vec3(mat.m[0][0], mat.m[0][1], mat.m[0][2]);
	_obbBox.axis[1] = Vec3(mat.m[1][0], mat.m[1][1], mat.m[1][2]);
	_obbBox.axis[2] = Vec3(mat.m[2][0], mat.m[2][1], mat.m[2][2]);

	for (int i = 0; i < 3; ++i)
		_obbBox.axis[i].Normalize();

	_obbBox.halfSize = Vec3(
		MySize.Width * 0.5f,
		MySize.Hight * 0.5f,
		MySize.Length * 0.5f
	);
}

OBB2D Tank::GetOBB2D() const
{
	return {
		Vec2(_obbBox.center.X, _obbBox.center.Z),
		{ Vec2(_obbBox.axis[0].X, _obbBox.axis[0].Z).GetNormalized(),
		  Vec2(_obbBox.axis[2].X, _obbBox.axis[2].Z).GetNormalized() },
		Vec2(_obbBox.halfSize.X, _obbBox.halfSize.Z)
	};
}

void Tank::Damage(int dmg)
{
	const int current = static_cast<int>(_hp);

	if (dmg >= current)
		_hp = 0;
	else
		_hp = static_cast<uint8>(current - dmg);
}

// ----------------------------------------------------------------
//  이동 예산 - 시간이 흐르는 만큼 누적, 이동한 만큼 감소.
// ----------------------------------------------------------------
bool Tank::CheckMoveSpeed(const Vec3& newPos, int64 nowMs)
{
	if (_lastMoveMs == 0)
	{
		// 첫 갱신. 기준점만 잡고 통과시킨다.
		ResetMoveCheck(nowMs, newPos);
		return true;
	}

	const float elapsedSec = static_cast<float>(nowMs - _lastMoveMs) / 1000.f;
	_lastMoveMs = nowMs;

	if (elapsedSec > 0.f)
	{
		_allowedMoveDist += MAX_TANK_SPEED * elapsedSec;
		if (_allowedMoveDist > MAX_ALLOWED_MOVE_DIST)
			_allowedMoveDist = MAX_ALLOWED_MOVE_DIST;   // 오래 멈춰 있다 한 번에 튀는 것 방지
	}

	/*  마지막으로 승인한 좌표와 비교
	    _myPos 는 Tank::Update 의 SyncPosFromMatrix 로 갱신*/
	const float dx = newPos.X - _lastAcceptedPos.X;
	const float dz = newPos.Z - _lastAcceptedPos.Z;

	// 수평 이동만 본다. 낙하는 중력이라 최대 속도와 무관하다.
	const float dist = std::sqrt(dx * dx + dz * dz);

	if (dist > _allowedMoveDist)
		return false;       // 초과 -> 거부

	_allowedMoveDist -= dist;
	_lastAcceptedPos = newPos;
	return true;
}

bool Tank::IsDead() 
{
	
	return _hp <= 0;

}

void Tank::SetSpawn(const Matrix4x4& mat, float PosinAngle, float PotapAngle)
{

	__super::SetTransform(mat);
	_potapAngle = PotapAngle;
	_posinAngle = PosinAngle;
	Spawn = true;
	_hp = 100;

	/*   스폰은 치트가 아님         */
	const Vec3 spawnPos(mat.m[3][0], mat.m[3][1], mat.m[3][2]);
	ResetMoveCheck(GetNowMs(), spawnPos);
}
