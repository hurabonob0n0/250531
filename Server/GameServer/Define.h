#pragma once

// 와이어 상수(TANK_WHEEL_COUNT 등)를 쓰기 위해. Protocol.h 는 <cstdint> 만 의존하므로 순환이 없다.
#include "Protocol.h"

#define PURE	= 0

#define ROBBY 9999
#define ROOM_ENTER_ERROR 9998
#define ROOM_CREATE_ERROR 9997      // 빈 방이 없어 방 생성 실패
#define TEAM_BLUE	false
#define TEAM_RED	true
/*----------------
	For Object
----------------*/

struct AirDropRect {
    float Left;
    float Right;
    float Top;
    float Bottom;
};




enum ObjectID {

	OBJ_TANK, OBJ_DRONE ,OBJ_BOMB,OBJ_WEAPON ,OBJ_END

};


enum RECV_Data {

	 DATA_TANK_MOVE, DATA_TANK_SHOT, DATA_TREE_DELETE 

};

struct Vec3
{
    float X;
    float Y;
    float Z;

    // 생성자
    Vec3() : X(0), Y(0), Z(0) {}
    Vec3(float x, float y, float z) : X(x), Y(y), Z(z) {}

    // 연산자 오버로딩
    Vec3 operator+(const Vec3& other) const {
        return Vec3(X + other.X, Y + other.Y, Z + other.Z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(X - other.X, Y - other.Y, Z - other.Z);
    }

    Vec3 operator*(float scalar) const {
        return Vec3(X * scalar, Y * scalar, Z * scalar);
    }

    Vec3 operator/(float scalar) const {
        return Vec3(X / scalar, Y / scalar, Z / scalar);
    }

    Vec3& operator+=(const Vec3& other) {
        X += other.X; Y += other.Y; Z += other.Z;
        return *this;
    }

    Vec3& operator-=(const Vec3& other) {
        X -= other.X; Y -= other.Y; Z -= other.Z;
        return *this;
    }

    // 벡터 크기
    float Length() const {
        return std::sqrt(X * X + Y * Y + Z * Z);
    }

    float LengthSq() const {
        return X * X + Y * Y + Z * Z;
    }

    // 정규화
    void Normalize() {
        float len = Length();
        if (len > 0.0f) {
            X /= len;
            Y /= len;
            Z /= len;
        }
    }

    Vec3 Normalized() const {
        float len = Length();
        if (len > 0.0f)
            return Vec3(X / len, Y / len, Z / len);
        return *this;
    }

    // 내적
    float Dot(const Vec3& rhs) const {
        return X * rhs.X + Y * rhs.Y + Z * rhs.Z;
    }

    static float Dot(const Vec3& a, const Vec3& b) {
        return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
    }

    // 외적
    Vec3 Cross(const Vec3& rhs) const {
        return Vec3(
            Y * rhs.Z - Z * rhs.Y,
            Z * rhs.X - X * rhs.Z,
            X * rhs.Y - Y * rhs.X
        );
    }

    static Vec3 Cross(const Vec3& a, const Vec3& b) {
        return a.Cross(b);
    }
};

struct Vec2
{
    float X, Y;

    Vec2() : X(0), Y(0) {}
    Vec2(float x, float y) : X(x), Y(y) {}

    float Length() const { return std::sqrt(X * X + Y * Y); }
    float LengthSq() const { return X * X + Y * Y; } 

    Vec2 GetNormalized() const
    {
        float len = Length();
        if (len == 0.f) return Vec2(0.f, 0.f);
        return Vec2(X / len, Y / len);
    }

    float Dot(const Vec2& rhs) const
    {
        return X * rhs.X + Y * rhs.Y;
    }

    Vec2 operator-(const Vec2& rhs) const
    {
        return Vec2(X - rhs.X, Y - rhs.Y);
    }

    Vec2 operator+(const Vec2& rhs) const
    {
        return Vec2(X + rhs.X, Y + rhs.Y);
    }

    Vec2 operator*(float scalar) const
    {
        return Vec2(X * scalar, Y * scalar);
    }
	

};

struct Size {
	float length_X;
	float length_Y;
	float length_Z;
};


/*  단위 쿼터니언. 회전 3자유도를 4개 값으로 담는다.
	와이어에서는 이걸 그대로 중계만 하고 서버는 해석하지 않는다
	(서버 로직은 위치만 쓴다 - GetPos). 행렬로 펴는 건 OBB 계산 때문이다. */
struct Quat
{
	float X = 0.f, Y = 0.f, Z = 0.f, W = 1.f;
};

struct Matrix4x4
{
	float m[4][4];

	static Matrix4x4 Identity()
	{
		Matrix4x4 result{};				/* ★ {} 로 전부 0 부터 채운다 */
		result.m[0][0] = 1.0f;
		result.m[1][1] = 1.0f;
		result.m[2][2] = 1.0f;
		result.m[3][3] = 1.0f;
		return result;
	}

	/*  ⚠️ 예전엔 여기서 'Matrix4x4 result;' 로 시작했다.
		POD 지역변수라 초기화가 안 되고, 아래에서 7개만 대입하므로
		나머지 9개(회전 부분 전부)가 스택 쓰레기값인 채로 클라에 전송됐다.
		클라가 곧 자기 상태로 덮어써서 눈에 안 띄었을 뿐이다.
		쿼터니언으로 바꾸면 그 쓰레기가 NaN 이 되어 탱크가 사라지므로 같이 고쳤다. */
	static Matrix4x4 CreateTranslation(float x, float y, float z)
	{
		Matrix4x4 result = Identity();
		result.m[3][0] = x;
		result.m[3][1] = y;
		result.m[3][2] = z;
		return result;
	}

	/*  회전(쿼터니언) + 위치로 행렬을 만든다.
		성분 배치는 DirectXMath 의 XMMatrixRotationQuaternion 과 같아야 한다
		(행 벡터 규약). 클라가 그 함수로 만든 행렬을 분해해 보내므로,
		여기서 규약이 어긋나면 탱크가 엉뚱한 자세로 그려진다. */
	static Matrix4x4 CreateFromQuatPos(const Quat& q, float x, float y, float z)
	{
		const float xx = q.X * q.X, yy = q.Y * q.Y, zz = q.Z * q.Z;
		const float xy = q.X * q.Y, xz = q.X * q.Z, yz = q.Y * q.Z;
		const float wx = q.W * q.X, wy = q.W * q.Y, wz = q.W * q.Z;

		Matrix4x4 result{};

		result.m[0][0] = 1.f - 2.f * (yy + zz);
		result.m[0][1] =       2.f * (xy + wz);
		result.m[0][2] =       2.f * (xz - wy);

		result.m[1][0] =       2.f * (xy - wz);
		result.m[1][1] = 1.f - 2.f * (xx + zz);
		result.m[1][2] =       2.f * (yz + wx);

		result.m[2][0] =       2.f * (xz + wy);
		result.m[2][1] =       2.f * (yz - wx);
		result.m[2][2] = 1.f - 2.f * (xx + yy);

		result.m[3][0] = x;
		result.m[3][1] = y;
		result.m[3][2] = z;
		result.m[3][3] = 1.f;

		return result;
	}
};


struct OBB
{
	Vec3 center;     
	Vec3 axis[3];    
	Vec3 halfSize;   
};


/*-----------------
	For Tank
-----------------*/


/* TANK_WHEEL_COUNT / TANK_WHEEL_SAG_RANGE 은 Protocol.h 에 있다(와이어 상수라 클라와 공유). */

struct Tank_INFO {

	/*  예전엔 Matrix4x4 를 통째로(64B) 주고받았다. 그 안에서
		마지막 열 (0,0,0,1) 16B 는 항상 상수고, 3자유도 회전에 9개를 쓰고 있었다.
		위치 3 + 쿼터니언 4 로 바꿔 28B 가 됐다.
		서버는 Rot 을 해석하지 않고 그대로 중계만 한다.                        */
	Vec3		Pos;
	Quat		Rot;

	float		PosinAngle;
	float		PotapAngle;
	uint8		TankHP;

	/* 바퀴 서스펜션. 실수 -0.4~0.4 를 -127~127 로 낮춰 담은 것 */
	int8		WheelSag[TANK_WHEEL_COUNT] = {};

};

struct Drone_INFO {

    Vec3	    DroneTransform;
    float       Yaw;
    float       Roll;
    float       Pitch;
    uint8		DroneHP;

};


struct OBB2D
{
	Vec2 center;
	Vec2 axis[2];      // x축, z축 방향의 단위 벡터
	Vec2 halfSize;     // 반 너비, 반 높이
};



/*-----------------
	For Weapon
-----------------*/

enum WEAPON_ID {

	WEAPON_NPOTAN, WEAPON_BOMB ,WEAPON_NBULLET, WEAPON_END

};


/*------------------
	For Room
------------------*/

struct Room_Data {

	
	unsigned char	MaxPlayer;
	unsigned char	CurPlayer;
	bool			isActive;
	unsigned char	RoomID;

};

/*------------------
	팀 false = Red
	팀 True  = Blue

	포지션 False = 조종수
	포지션 True = 포수

	

-------------------*/

struct Room_Ready_Data {
	
	uint8	PlayerID;
	uint8   Position;
	bool	Team;
	bool	IsReady = false;
	
};





/*-----------------
	For Delete
-----------------*/




template<typename T>
void Safe_Delete(T& Temp)
{
	if (Temp)
	{
		delete Temp;
		Temp = nullptr;
	}
}






/*------------------
	For Protocol
------------------*/


