#pragma once
#include "RenderObject.h"
#include "MyPhysicsEngine.h"
#include <functional>

BEGIN(Engine)
class CModel;
class CBBinding;
class CVIBuffer_Quad;
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
	void Set_Team(int Team);

public:
	void Set_PotapRotation(float fDegree);
	void Set_PoSinpRotation(float fDegree);
	void Set_ShotDir(XMVECTOR Vec);
	void Set_ShotMatrix(_matrix mat);

private:
	CModel* m_VIBuffer;
	
	_matrix ShotMatrix;
	XMVECTOR vShotDir;
	float   m_fPotapRotation = 0.f;			//이게 카메라가 주는 회전값
	float   m_fPosinRotation = 0.f;			//이게 카메라가 주는 회전값
	float	m_fCamPotapRot = 0.f;
	float	m_fCamPosinRot = 0.f;

	float   m_TestZ = -512.f;
	float	m_TestX = -512.f;
	bool	KeyInput = false;

	_uint matindex= 0;

private:
	void RotPotap_And_Posin(float fTimeDelta);

public:
	void set_MyPlayer() {
		_myPlayer = true;
		Set_Team(3);
	}
	void SendMyStateToServer();
	void SendShootDataToServer();

	void Set_OtherPlayerState(_float4x4 mat, float PotapRot, float PosinRot);


private:		//For PosinCrosshair
	CBBinding*				m_CBBindingQuad;
	CVIBuffer_Quad*			m_VIBufferQuad;
	CRenderer::RENDERGROUP	m_RGQuad = CRenderer::RENDERGROUP::RG_UI;
	CTransform*				m_QuadWorldTransform;
	CTransform*				m_QuadTexTransform;
	//bool					m_isQuadTurn = false;
	float					m_fSameTime = 0.f;
	bool					m_isFPS = false;

private:		//For PosinCrosshair
	void Initialize_For_PosinQuad();
	void Tick_For_Posin_Image(float fTimeDelta);
	void Render_For_Posin_Image();


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