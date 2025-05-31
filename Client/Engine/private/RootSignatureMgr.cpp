#include "RootSignatureMgr.h"

IMPLEMENT_SINGLETON(CRootSignatureMgr)

void CRootSignatureMgr::Register(const std::string& name, CRootSignature* RS)
{
    if (mRootSignatures.find(name) != mRootSignatures.end())
        return;
    mRootSignatures[name] = RS;
}

ID3D12RootSignature* CRootSignatureMgr::Get(const std::string& name) const
{
    return mRootSignatures.find(name)->second->Get();
}

void CRootSignatureMgr::Remove(const std::string& name)
{
    mRootSignatures.erase(name);
}

void CRootSignatureMgr::Free()
{
    for (auto& RootSig : mRootSignatures)
        Safe_Release(RootSig.second);

    mRootSignatures.clear();
}
