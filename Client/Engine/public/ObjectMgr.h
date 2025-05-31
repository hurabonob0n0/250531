#pragma once
#include "Base.h"
#include "GameObject.h"
#include "RenderObject.h"
#include "Camera.h"

BEGIN(Engine)

class ENGINE_DLL CObjectMgr : public CBase
{
    DECLARE_SINGLETON(CObjectMgr)

private:
    CObjectMgr();
    virtual ~CObjectMgr() = default;

public:
    typedef vector<CGameObject*> Layer;

    HRESULT AddObject(string PrototypeTag, string layerTag, void* pArg);

    HRESULT Add_PrototypeObject(string Prototypename, CGameObject* pGameObject);

    HRESULT RemoveObject(const string& layerTag, CGameObject* pGameObject);

public:
    void Set_Pos_For_Server(string LayerName, _uint ID, float* pos) {
        m_Layers[LayerName][ID]->Set_Position(pos);
    }

    void Update(const float& fTimeDelta);
    void LateUpdate(const float& fTimeDelta);

    CGameObject* GetGameObject(string LayerName, _uint ID) {
        return  m_Layers[LayerName][ID];
    };

private:
    unordered_map<string, Layer> m_Layers;
    unordered_map<string, CGameObject*> m_Prototypes;



public:
    void Free() override;
};

END