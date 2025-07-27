#include "..\Public\Random_Manager.h"
#include <chrono>


IMPLEMENT_SINGLETON(CRandom_Manager)

CRandom_Manager::CRandom_Manager()
{
}

_int CRandom_Manager::Get_RandomI(_int iStart, _int iEnd)
{
	uniform_int_distribution<_int> m_iDistribution(iStart, iEnd);

	return m_iDistribution(m_Generator);
}

float CRandom_Manager::Get_RandomF(float Start, float End)
{
	uniform_real_distribution<_float> m_iDistribution(Start, End);

	return m_iDistribution(m_Generator);
}

void CRandom_Manager::Tick()
{
	m_Generator.seed(static_cast<unsigned int>(chrono::system_clock::now().time_since_epoch().count()));

	return;
}

void CRandom_Manager::Free()
{
}
