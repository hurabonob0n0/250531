#pragma once
#include "Define.h"

class MyBmp
{
public:
	MyBmp();
	~MyBmp();

public:
	HDC			Get_MemDC() { return m_hMemDC; }
	void		Load_Bmp(const TCHAR* pFilePath,HWND handle);
	void		Release();

private:
	HDC			m_hMemDC;

	HBITMAP		m_hBitmap;
	HBITMAP		m_hOldBmp;
};
