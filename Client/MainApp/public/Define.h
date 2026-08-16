#pragma once


#define PURE	= 0
#define PI		3.14f
#define VK_MAX	0xff

#define		OBJ_NOEVENT			0
#define		OBJ_DEAD			1
#define		GAMESTART			2

enum LEVEL_ID {LEVEL_MENU,LEVEL_ROBBY,LEVEL_ROOMLIST ,LEVEL_ROOM,LEVEL_END};
enum OBJ_ID { OBJ_IMAGE, OBJ_BUTTON,OBJ_ROOMLISTBAR ,OBJ_END };
enum BUTTON_ID { BUTTON_SINGLEPLAY, BUTTON_MULTIPLAY, BUTTON_ROOMLIST, BUTTON_CREATE, BUTTON_JOIN, BUTTON_REFRESH,
	BUTTON_EXIT,BUTTON_READY,BUTTON_START, BUTTON_END };

template<typename T>
void Safe_Delete(T& Temp)
{
	if (Temp)
	{
		delete Temp;
		Temp = nullptr;
	}
}


typedef struct tagInfo
{
	float	fX;		// 중점 X
	float	fY;		// 중점 Y
	float	fCX;	// 가로 사이즈
	float	fCY;	// 세로 사이즈

}INFO;



struct Vector2 {
	float x;
	float y;

	Vector2() : x(0), y(0) {}
	Vector2(float x, float y) : x(x), y(y) {}


	Vector2 normalize() const {
		float length = sqrt(x * x + y * y);
		if (length != 0) {
			return Vector2(x / length, y / length);
		}
		return Vector2(0, 0);
	}
};

//typedef struct Rect {
//
//	POINT LT;
//	POINT RB;
//
//}RECT;

typedef struct tagFrame
{
	int		iFrameStart;
	int		iFrameEnd;
	int		iMotion;
	DWORD	dwSpeed;
	DWORD	dwTime;

}FRAME;

class CDeleteObj
{
public:
	template<typename T>
	void operator()(T& Temp)
	{
		if (Temp)
		{
			delete Temp;
			Temp = nullptr;
		}
	}
};

class CDeleteMap
{
public:
	template<typename T>
	void operator()(T& MyPair)
	{
		if (MyPair.second)
		{
			delete MyPair.second;
			MyPair.second = nullptr;
		}
	}
};


class CTagFinder
{
public:
	CTagFinder(const TCHAR* pKey) : m_pKey(pKey) {}

public:
	template<typename T>
	bool	operator()(T& Pair)
	{
		if (!lstrcmp(m_pKey, Pair.first))
			return true;

		return false;
	}

private:
	const TCHAR* m_pKey;
};




extern HWND g_hWnd;