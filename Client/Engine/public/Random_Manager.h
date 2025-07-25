#pragma once

#include "Base.h"
#include <random>

BEGIN(Engine)

class CRandom_Manager final : public CBase
{
	DECLARE_SINGLETON(CRandom_Manager)

private:
	CRandom_Manager();
	virtual ~CRandom_Manager() = default;

private:
	mt19937 m_Generator;

public:
	_int Get_RandomI(_int iStart, _int iEnd);
	float Get_RandomF(float Start, float End);

public:
	HRESULT Initialize();
	void Tick();

public:
	virtual void	Free();
};

END