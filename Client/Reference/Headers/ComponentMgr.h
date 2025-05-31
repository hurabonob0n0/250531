#pragma once
#include "Base.h"
#include "Component.h"
#include "VIBuffer_Geos.h"
#include "Transform.h"
#include "Model.h"
#include "VIBuffer_Terrain.h"
#include "CBBinding.h"

class CComponentMgr : public CBase
{
    DECLARE_SINGLETON(CComponentMgr)

private:
    CComponentMgr() = default;
    virtual ~CComponentMgr() = default;

public:
    // 원본 컴포넌트 등록
    HRESULT AddPrototype(const string& prototypeTag, CComponent* pPrototype)
    {
        if (m_Prototypes.find(prototypeTag) != m_Prototypes.end())
            return E_FAIL; // 이미 등록됨

        m_Prototypes[prototypeTag] = pPrototype;
        return S_OK;
    }

    // 원본 컴포넌트로부터 Clone 반환
    CComponent* GetComponent(const string& prototypeTag, void* pArg = nullptr) const
    {
        auto iter = m_Prototypes.find(prototypeTag);
        if (iter != m_Prototypes.end())
            return iter->second->Clone(pArg); // 복사본 반환

        return nullptr;
    }

    CComponent* GetPrototype(const string& prototypeTag) const
    {
        auto iter = m_Prototypes.find(prototypeTag);
        if (iter != m_Prototypes.end())
            return iter->second; // 

        return nullptr;
    }

    // 등록된 모든 원본 해제
    void Free() override
    {
        for (auto& pair : m_Prototypes)
        {
            Safe_Release(pair.second); // or delete pair.second;
        }
        m_Prototypes.clear();
    }

private:
    unordered_map<string, CComponent*> m_Prototypes;
};

