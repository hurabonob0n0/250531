#pragma once
#include "Engine_Defines.h"

BEGIN(Engine)

class ENGINE_DLL CBase abstract
{
protected:
	CBase();
	virtual ~CBase();

public:
	unsigned long AddRef();
	unsigned long Release();

protected:
	unsigned long			m_dwRefCnt = { 0 };

public:
	virtual void Free() = 0;

};

END
