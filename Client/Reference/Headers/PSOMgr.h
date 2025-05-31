#pragma once
#include "Base.h"
#include "PSO.h"

BEGIN(Engine)

class CPSOMgr : public CBase
{
	DECLARE_SINGLETON(CPSOMgr)

public:
	CPSOMgr() = default;
	virtual ~CPSOMgr() = default;

public:
	// PSO 추가 (InputLayout 타입 추가!)
	HRESULT AddPSO(const string& PSOName, CPSO* PSOInstance);

	// PSO 정보 얻기
	CPSO* GetPSOObj(const string& PSOName) const;

	// PSO 버퍼 정보
	ID3D12PipelineState* Get(const string& PSOName) const;

	// 공간 확보.
	void Reserve() { m_PSOs.reserve(20); }

private:
	unordered_map<string, CPSO*> m_PSOs;

public:
	virtual void Free() override;
};

END
