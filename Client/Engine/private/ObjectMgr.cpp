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
        for (auto it = pair.second.begin(); it != pair.second.end();)
        {
            CGameObject* obj = *it;
            if (!obj) { continue; }

            // 보통은 LateTick 먼저 돌리고, 그 안에서 죽음 플래그가 설령 켜져도 다음 줄에서 바로 반영
            obj->LateTick(fTimeDelta);

            if (obj->Get_Dead()) {
                Safe_Release(obj);
                it = pair.second.erase(it);  // erase가 다음 원소 이터레이터를 반환
            }
            else {
                ++it;
            }
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