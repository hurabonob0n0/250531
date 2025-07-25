#pragma once
#include "Define.h"

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

public:
	virtual void Initialize()	PURE;
	virtual int	 Update()		PURE;
	virtual void Late_Update()	PURE;
	virtual void Render(HDC hDC)PURE;
	virtual void Release(void)	PURE;

public:
	void		Set_Index(short x, short y)
	{
		m_PosIndex.x = x;
		m_PosIndex.y = y;
	}
	void		Set_Pos(float _fX, float _fY)
	{
		m_tInfo.fX = _fX;
		m_tInfo.fY = _fY;
	}
	void		Set_Dead() { m_bDead = true; }
	void		Set_Angle(float _fAngle) { m_fAngle = _fAngle; }
	void		Set_Size(float fCX, float fCY) { m_tInfo.fCX = fCX, m_tInfo.fCY = fCY; };

	void		Set_Target(GameObject* _pTarget) { m_pTarget = _pTarget; }

	void		Set_X(float _fX) { m_tInfo.fX += _fX; }
	void		Set_Y(float _fY) { m_tInfo.fY += _fY; }
	void		Set_FrameKey(const TCHAR* pFrameKey) { m_pFrameKey = pFrameKey; }
	void		Set_Frame(int iFrameEnd, DWORD FrameSpeed) {

		m_tFrame.iFrameStart = 0;
		m_tFrame.iFrameEnd = iFrameEnd;
		m_tFrame.dwSpeed = FrameSpeed;
		m_tFrame.dwTime = GetTickCount64();
	};
	const RECT* Get_Rect() { return &m_tRect; }
	INFO		Get_Info() { return m_tInfo; }
	bool		Get_Dead() { return m_bDead; }
	int			Get_HP() { return _Hp; };
	void		Set_Hp(int iHp) { _Hp = iHp; }
	int			Get_MP() { return _Mp; };
	void		Set_Mp(int iMp) { _Mp = iMp; }
	int			Get_Atk() { return m_iAttack; }
	
	void		Set_Exp(int iExp) { _Exp = iExp; };
	int			Get_Exp() { return _Exp; };
	void		Set_Level(int iLevel) { _level = iLevel; };
	int			Get_Level() { return _level; };



protected:
	void		Update_Rect();
	void		Move_Frame();

protected:
	GameObject* m_pTarget;
	INFO		m_tInfo;
	RECT		m_tRect;
	FRAME		m_tFrame;

	Vector2		m_PosIndex;
	bool		m_bDead;



	float		m_fSpeed;
	float		m_fAngle;
	long long	_id;
	char		_name[20];
	short		_max_hp;
	int			_Hp;
	int			_Mp;
	int			_Exp;
	short		_level;
	int			m_iAttack;
	int			m_iFrameCnt;
	bool		_isAlive = true;

	const TCHAR* m_pFrameKey;

};
