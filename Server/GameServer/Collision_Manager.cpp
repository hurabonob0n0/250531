#include "pch.h"
#include "Define.h"
#include "Collision_Manager.h"
#include "ObjectManager.h"
#include "Terrain_Manager.h"



bool CollisionManager::CheckCollision_OBB2D_Circle(const Vec2& circleCenter, float radius, const Vec2& boxCenter, const Vec2 axes[2], const Vec2& halfSize)
{
	Vec2 dir = circleCenter - boxCenter;

	float localX = dir.Dot(axes[0]);
	float localY = dir.Dot(axes[1]);

	float clampedX = std::clamp(localX, -halfSize.X, halfSize.X);
	float clampedY = std::clamp(localY, -halfSize.Y, halfSize.Y);

	Vec2 closestPoint = boxCenter + axes[0] * clampedX + axes[1] * clampedY;
	Vec2 diff = closestPoint - circleCenter;

	return diff.LengthSq() <= radius * radius;
}

bool CollisionManager::CheckCollision_Point_OBB3D(const Vec3& point, const OBB& obb)
{
	Vec3 dir = point - obb.center;

	for (int i = 0; i < 3; ++i)
	{
		float projection = dir.Dot(obb.axis[i]);
		float extent = (i == 0) ? obb.halfSize.X : (i == 1) ? obb.halfSize.Y : obb.halfSize.Z;
		if (fabs(projection) > extent)
			return false;

	}

	return true;
}

bool CollisionManager::Check_Terrain_Collision(GameObject* object)
{
	const Vec3& pos = object->GetPos();
	float terrainHeight = Terrain_Manager::GetInstance().Get_Height(pos.X, pos.Z);
	return pos.Y <= terrainHeight;

}

/*  선분 OBB
    startOnAxis : 선분 시작점이 그 축 위 어디에 있나. 상자 중심이 0
    moveOnAxis  : 선분이 t 0 -> 1 동안 그 축 방향으로 얼마나 가나
    halfExtent  : 그 축 방향으로 잰 상자의 반 크기
  */
bool CollisionManager::CheckCollision_Segment_OBB3D(const Vec3& startPos, const Vec3& endPos,
													const OBB& obb, Vec3* pOutHit)
{
	const Vec3 moveVector      = endPos - startPos;
	const Vec3 startFromCenter = startPos - obb.center;

	const float halfExtents[3] = { obb.halfSize.X, obb.halfSize.Y, obb.halfSize.Z };

	float tOverlapStart = 0.f;
	float tOverlapEnd   = 1.f;

	for (int i = 0; i < 3; ++i)
	{
		const float startOnAxis = startFromCenter.Dot(obb.axis[i]);   // 시작점의 이 축 좌표
		const float moveOnAxis  = moveVector.Dot(obb.axis[i]);        // 이 축으로 가는 거리
		const float halfExtent  = halfExtents[i];

		if (fabsf(moveOnAxis) < 1e-6f)
		{
			if (startOnAxis < -halfExtent || startOnAxis > halfExtent)
				return false;
			continue;
		}

		float tEnter = (-halfExtent - startOnAxis) / moveOnAxis;      // 가까운 벽에 닿는 시각
		float tExit  = ( halfExtent - startOnAxis) / moveOnAxis;      // 먼 벽을 벗어나는 시각

		if (tEnter > tExit)                                           // 뒤로 가는 축이면 순서가 뒤집힌다
		{
			const float temp = tEnter;
			tEnter = tExit;
			tExit  = temp;
		}

		if (tEnter > tOverlapStart) tOverlapStart = tEnter;           // 교집합의 시작을 늦춘다
		if (tExit  < tOverlapEnd)   tOverlapEnd   = tExit;            // 교집합의 끝을 당긴다

		if (tOverlapStart > tOverlapEnd)                              // 겹치는 시간대가 없다
			return false;
	}

	if (pOutHit)
		*pOutHit = startPos + moveVector * tOverlapStart;             // 처음 닿은 지점

	return true;
}

// ================================================================
//  지형 + 선 충돌

// ================================================================
bool CollisionManager::CheckCollision_Segment_Terrain(const Vec3& p0, const Vec3& p1,
													  Vec3* pOutHit)
{
	Terrain_Manager& terrain = Terrain_Manager::GetInstance();

	// 시작점이 이미 땅 아래면 그 자리가 착탄점이다.
	if (p0.Y <= terrain.Get_Height(p0.X, p0.Z))
	{
		if (pOutHit) *pOutHit = p0;
		return true;
	}

	const Vec3  dir = p1 - p0;
	const float len = dir.Length();

	int steps = static_cast<int>(len) + 1;      // 약 1유닛마다
	if (steps > 64) steps = 64;                 // dt 가 튀어도 상한을 둔다

	float tAbove = 0.f;     // 마지막으로 '지형 위' 였던 지점

	for (int i = 1; i <= steps; ++i)
	{
		const float t = static_cast<float>(i) / static_cast<float>(steps);
		const Vec3  p = p0 + dir * t;

		if (p.Y > terrain.Get_Height(p.X, p.Z))
		{
			tAbove = t;
			continue;
		}

		// 위 와 아래 사이에 지면이 있다
		float lo = tAbove, hi = t;
		for (int k = 0; k < 8; ++k)
		{
			const float mid = (lo + hi) * 0.5f;
			const Vec3  pm  = p0 + dir * mid;

			if (pm.Y <= terrain.Get_Height(pm.X, pm.Z)) hi = mid;
			else                                        lo = mid;
		}

		if (pOutHit) *pOutHit = p0 + dir * hi;
		return true;
	}

	return false;
}

bool CollisionManager::CheckCollision_Point_Sphere(const Vec3& point, const Vec3& center, float radius)
{
	Vec3 diff = point - center;
	return diff.LengthSq() <= radius * radius;
}

bool CollisionManager::CheckCollision_Point_Sphere2D(const Vec3& point, const Vec3& center, float radius)
{
	Vec2 point2D = { point.X, point.Z };
	Vec2 center2D = { center.X, center.Z };

	Vec2 diff = point2D - center2D;
	return diff.LengthSq() <= radius * radius;
}