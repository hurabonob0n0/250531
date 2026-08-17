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

// ================================================================
//  선 - OBB
//  OBB 를 세 축의 평행판 세 쌍이 겹친 것으로 본다
// ================================================================
bool CollisionManager::CheckCollision_Segment_OBB3D(const Vec3& p0, const Vec3& p1,
													const OBB& obb, Vec3* pOutHit)
{
	const Vec3 dir = p1 - p0;           // 방향 + 길이
	const Vec3 m   = p0 - obb.center;   // OBB 중심 기준 시작점

	const float half[3] = { obb.halfSize.X, obb.halfSize.Y, obb.halfSize.Z };

	float tMin = 0.f;
	float tMax = 1.f;

	for (int i = 0; i < 3; ++i)
	{
		const float e = m.Dot(obb.axis[i]);     // 시작점의 이 축 좌표
		const float f = dir.Dot(obb.axis[i]);   // 이 축으로 얼마나 가는가
		const float h = half[i];

		if (fabsf(f) < 1e-6f)
		{
			// 이 축과 거의 평행하다. 시작점이 이미 판 밖이면 영영 못 만난다.
			if (e < -h || e > h)
				return false;
			continue;
		}

		float t1 = (-h - e) / f;
		float t2 = ( h - e) / f;
		if (t1 > t2) { const float tmp = t1; t1 = t2; t2 = tmp; }

		if (t1 > tMin) tMin = t1;
		if (t2 < tMax) tMax = t2;

		if (tMin > tMax)
			return false;       // 교집합이 비었다
	}

	if (pOutHit)
		*pOutHit = p0 + dir * tMin;

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