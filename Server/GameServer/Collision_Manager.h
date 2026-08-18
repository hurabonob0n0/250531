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
	//  선 판정 - 빠른 발사체가 틱 사이에 물체를 통과하는 것을 막는다
	// ================================================================
	bool CheckCollision_Segment_OBB3D(const Vec3& startPos, const Vec3& endPos,
									  const OBB& obb, Vec3* pOutHit = nullptr);

	bool CheckCollision_Segment_Terrain(const Vec3& p0, const Vec3& p1,
										Vec3* pOutHit = nullptr);

	bool Check_Terrain_Collision(GameObject* GameObejct);

	bool CheckCollision_Point_Sphere(const Vec3& point, const Vec3& center, float radius);

	bool CheckCollision_Point_Sphere2D(const Vec3& point, const Vec3& center, float radius);


};
