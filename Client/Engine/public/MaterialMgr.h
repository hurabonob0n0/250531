#pragma once
#include "Base.h"

//BEGIN(Engine)

//class CMaterialMgr : public CBase
//{
//	DECLARE_SINGLETON(CMaterialMgr)
//private:
//	CMaterialMgr() = default;
//	virtual~CMaterialMgr() = default;
//
//public:
//	int Add_Material(string matName, MaterialData matInstance);
//
//public:
//	_uint Get_Mat_Index(string matName);
//
//	_uint Get_Mat_Size();
//
//public:
//	void Set_Materials();
//
//public:
//	void Update_Mats();
//
//public:
//	void Resize();
//
//private:
//	unordered_map<string, MaterialData> m_MatMap;
//
//public:
//	void Free();
//};
//
//END

BEGIN(Engine)

class CMaterialMgr : public CBase
{
	DECLARE_SINGLETON(CMaterialMgr)
private:
	CMaterialMgr() = default;
	virtual~CMaterialMgr() = default;

public:
	_uint Add_Material(string matName, MaterialData matInstance);

public:
	_uint Get_Mat_Index(string matName);
	_uint Get_Mat_Size();

public:
	void Set_Materials();
	void Update_Mats();

public:
	void Resize();

private:
	// 이름과 데이터를 한 쌍으로 묶어 vector에서 관리
	vector<pair<string, MaterialData>> m_Materials;

public:
	void Free();
};

END