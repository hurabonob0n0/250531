#include "PSOMgr.h"

IMPLEMENT_SINGLETON(CPSOMgr)

HRESULT CPSOMgr::AddPSO(const string& PSOName, CPSO* PSOInstance)
{
	if (m_PSOs.find(PSOName) != m_PSOs.end())
		return E_FAIL; // 이미 있음

	m_PSOs[PSOName] = PSOInstance;
	return S_OK;
}

CPSO* CPSOMgr::GetPSOObj(const string& PSOName) const
{
	auto iter = m_PSOs.find(PSOName);
	if (iter != m_PSOs.end())
		return iter->second;
	return nullptr;
}

ID3D12PipelineState* CPSOMgr::Get(const string& PSOName) const
{
	CPSO* pso = GetPSOObj(PSOName);
	if (pso)
		return pso->Get();
	return nullptr;
}

void CPSOMgr::Free()
{
	for (auto& pair : m_PSOs)
	{
		Safe_Release(pair.second); // delete pair.second도 가능
	}
	m_PSOs.clear();
}
