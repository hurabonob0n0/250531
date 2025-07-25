#include "Client_pch.h"
#include "Bmp_Manager.h"
#include "MyBmp.h"


Bmp_Manager* Bmp_Manager::m_pInstance = nullptr;

Bmp_Manager::Bmp_Manager()
{
}

Bmp_Manager::~Bmp_Manager()
{
	Release();
}

void Bmp_Manager::Insert_Bmp(const TCHAR* pFilePath, const TCHAR* pImgKey,HWND Handle)
{

	auto	iter = find_if(m_mapBit.begin(), m_mapBit.end(), CTagFinder(pImgKey));

	if (iter == m_mapBit.end())
	{
		MyBmp* pBmp = new MyBmp;
		pBmp->Load_Bmp(pFilePath, Handle);

		m_mapBit.insert({ pImgKey, pBmp });
	}

}

HDC Bmp_Manager::Find_Img(const TCHAR* pImgKey)
{
	auto	iter = find_if(m_mapBit.begin(), m_mapBit.end(), CTagFinder(pImgKey));

	if (iter == m_mapBit.end())
		return nullptr;

	return iter->second->Get_MemDC();
}

void Bmp_Manager::Release(void)
{
	for_each(m_mapBit.begin(), m_mapBit.end(), CDeleteMap());
	m_mapBit.clear();
}
