#pragma once

struct Vec2;
struct Vec3;
struct OBB;

class GameObject;



class CollisionManager
{
private:
	CollisionManager() = default;
	~CollisionManager() = default;

public:
	static CollisionManager* GetInstance()
	{
		static CollisionManager instance;
		return &instance;
	}

	// 2D OBB vs Circle
	bool CheckCollision_OBB2D_Circle(const Vec2& circleCenter, float radius, const Vec2& boxCenter, const Vec2 axes[2], const Vec2& halfSize);

	// 3D Point vs OBB
	bool CheckCollision_Point_OBB3D(const Vec3& point, const OBB& obb);

	// ================================================================
	//  선분(스윕) 판정 - 빠른 발사체가 틱 사이에 물체를 통과하는 것을 막는다
	//
	//  ★ 히트스캔이 아니다.
	//    쏘는 순간 무한대로 광선을 쏘는 게 아니라, 매 틱 "직전 위치 → 현재 위치"
	//    선분만 본다. 총알은 그대로 날아가고, 이산 표본 사이의 틈만 메운다.
	//    한 틱 선분 길이는 총알 2.5 유닛(150 u/s ÷ 60), 폭탄 0.83 유닛이다.
	//
	//  pOutHit 에는 처음 닿은 지점을 돌려준다(시작점이 이미 안이면 시작점).
	//  착탄 이펙트를 표면에 그리려면 이 값을 써야 한다.
	// ================================================================
	bool CheckCollision_Segment_OBB3D(const Vec3& p0, const Vec3& p1,
									  const OBB& obb, Vec3* pOutHit = nullptr);

	bool CheckCollision_Segment_Terrain(const Vec3& p0, const Vec3& p1,
										Vec3* pOutHit = nullptr);

	bool Check_Terrain_Collision(GameObject* GameObejct);

	bool CheckCollision_Point_Sphere(const Vec3& point, const Vec3& center, float radius);

	bool CheckCollision_Point_Sphere2D(const Vec3& point, const Vec3& center, float radius);


};
