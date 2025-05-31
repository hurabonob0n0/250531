#pragma once
#include "RenderObject.h"
#include "MyPhysicsEngine.h"

BEGIN(Engine)
class CModel;
END


BEGIN(Client)

class CTank : public CRenderObject
{
private:
	CTank();
	CTank(CTank& rhs);
	virtual ~CTank() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CModel* m_VIBuffer;
	bool	KeyInput = false;
	float   m_fPotapRotation = 0.f;
	float   m_fPosinRotation = 0.f;

public:
	void set_MyPlayer() {
		_myPlayer = true;
	}
private:
	bool _myPlayer = false;

private:
	MyPhysicsEngine::CMyPhysicsEngine* m_pPhysicsEngine = nullptr;
	MyPhysicsEngine::CMyPhysicsEngine::TankControlState m_TankConsrolState{};


public:
	void Free() override;
	static CTank* Create();
	CRenderObject* Clone(void* pArg);
};

END