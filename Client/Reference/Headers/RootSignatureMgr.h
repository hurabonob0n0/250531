#pragma once
#include "Base.h"
#include "RootSignature.h"

BEGIN(Engine)



class ENGINE_DLL CRootSignatureMgr : public CBase
{
    DECLARE_SINGLETON(CRootSignatureMgr)
private:
    CRootSignatureMgr() = default;
    virtual ~CRootSignatureMgr() = default;

public:
    void Register(const std::string& name,CRootSignature* RS);

    // 루트 시그니처 가져오기
    ID3D12RootSignature* Get(const std::string& name) const;

    // 루트 시그니처 제거
    void Remove(const std::string& name);

public:
    void Free() override;

private:
    std::unordered_map<std::string, CRootSignature*> mRootSignatures;
};

END