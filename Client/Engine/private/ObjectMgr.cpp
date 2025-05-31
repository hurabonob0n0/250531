#include "ObjectMgr.h"

IMPLEMENT_SINGLETON(CObjectMgr)

CObjectMgr::CObjectMgr()
{
}

HRESULT CObjectMgr::AddObject(string PrototypeTag, string layerTag,void* pArg)
{
    auto iter = m_Prototypes.find(PrototypeTag);
    if (iter == m_Prototypes.end())
        return E_FAIL;

    m_Layers[layerTag].push_back(iter->second->Clone(pArg));

    return S_OK;
}

HRESULT CObjectMgr::Add_PrototypeObject(string Prototypename, CGameObject* pGameObject)
{
    auto iter = m_Prototypes.find(Prototypename);
    if (iter != m_Prototypes.end())
        return E_FAIL;

    m_Prototypes[Prototypename] = pGameObject;

    return S_OK;
}

HRESULT CObjectMgr::RemoveObject(const string& layerTag, CGameObject* pGameObject)
{
    auto iter = m_Layers.find(layerTag);
    if (iter == m_Layers.end())
        return E_FAIL;

    Layer& layer = iter->second;
    auto objIter = std::find(layer.begin(), layer.end(), pGameObject);

    if (objIter != layer.end())
    {
        Safe_Release(*objIter);  // 참조 카운트 감소 (혹은 delete)
        layer.erase(objIter);
        return S_OK;
    }

    return E_FAIL;
}

void CObjectMgr::Update(const float& fTimeDelta)
{
    for (auto& pair : m_Layers)
    {
        for (auto& obj : pair.second)
        {
            if (obj)
                obj->Tick(fTimeDelta);
        }
    }
}

void CObjectMgr::LateUpdate(const float& fTimeDelta)
{
    for (auto& pair : m_Layers)
    {
        for (auto& obj : pair.second)
        {
            if (obj)
                obj->LateTick(fTimeDelta);
        }
    }
}

void CObjectMgr::Free()
{
    for (auto& pair : m_Layers)
    {
        for (auto& obj : pair.second)
        {
            Safe_Release(obj);  // or delete obj;
        }
        pair.second.clear();
    }

    for (auto& obj : m_Prototypes)
        Safe_Release(obj.second);

    m_Layers.clear();
}