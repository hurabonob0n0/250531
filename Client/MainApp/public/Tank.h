#pragma once
#include "RenderObject.h"
#include "MyPhysicsEngine.h"
#include <functional>

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

public:
	void Set_PotapRotation(float fDegree);
	void Set_PoSinpRotation(float fDegree);
	void Set_ShotDir(XMVECTOR Vec);
	void Set_ShotMatrix(_matrix mat);
private:
	CModel* m_VIBuffer;
	
	_matrix ShotMatrix;
	XMVECTOR vShotDir;
	float   m_fPotapRotation = 0.f;
	float   m_fPosinRotation = 0.f;

	float   m_TestZ = -512.f;
	float	m_TestX = -512.f;
	bool	KeyInput = false;

	_uint matindex= 0;

public:
	void set_MyPlayer() {
		_myPlayer = true;
	}
	void SendMyStateToServer();
	void SendShootDataToServer();

	void Set_OtherPlayerState(_float4x4 mat, float PotapRot, float PosinRot);



public:
	bool _myPlayer = false;

private:
	MyPhysicsEngine::CMyPhysicsEngine* m_pPhysicsEngine = nullptr;
	MyPhysicsEngine::CMyPhysicsEngine::TankControlState m_TankConsrolState{};

public:
	void PushBulletMatrix(const _matrix& mat);
	void PopAllBulletMatrix(std::function<void(const _matrix&)> processFunc);

private:
	std::queue<_matrix> BulletQueue;
	std::mutex BulletQueueMutex;

public:
	void Free() override;
	static CTank* Create();
	CRenderObject* Clone(void* pArg);



};

END