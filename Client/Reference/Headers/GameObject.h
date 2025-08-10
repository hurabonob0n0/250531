#pragma once
#include "Base.h"
#include "Transform.h"

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	CGameObject();
	CGameObject(CGameObject& rhs);
	virtual ~CGameObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta) = 0;
	virtual void LateTick(float fTimeDelta) = 0;

	void Set_Position(float* position) {
		_vector vec = XMVectorSet(position[0], position[1], position[2], 1.f);
		m_TransformCom->Set_State(CTransform::STATE_POSITION, vec);
	}

public:
	bool Get_Dead() { return isDead; }

protected:
	CTransform* m_TransformCom = {};
	class CGameInstance* m_GameInstance;

protected:
	bool isDead = false;

public:
	CComponent* Get_Object_Component(string ComName) { return m_Components[ComName]; };

protected:
	unordered_map<string, class CComponent*> m_Components;

public:
	void Free() override;
	virtual CGameObject* Clone(void* pArg) = 0;
};

END