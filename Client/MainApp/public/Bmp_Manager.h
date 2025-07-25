#pragma once
#include "Define.h"
class MyBmp;

class Bmp_Manager
{
private:
	Bmp_Manager();
	~Bmp_Manager();

public:
	void		Insert_Bmp(const TCHAR* pFilePath, const TCHAR* pImgKey,HWND handle);
	HDC			Find_Img(const TCHAR* pImgKey);
	void		Release(void);

public:
	static	Bmp_Manager* Get_Instance()
	{
		if (!m_pInstance)
		{
			m_pInstance = new Bmp_Manager;
		}
		return m_pInstance;
	}

	static void			Destroy_Instance()
	{
		if (m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = nullptr;
		}
	}

private:
	static	Bmp_Manager* m_pInstance;

	map<const TCHAR*, MyBmp*>		m_mapBit;
};

