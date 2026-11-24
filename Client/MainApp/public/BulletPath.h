#pragma once
#include "Client_Defines.h"
#include "RenderObject.h"

BEGIN(Engine)
class CVIBuffer_Geos;
class CBBinding;
END

BEGIN(Client)

class CBulletPath : public CRenderObject
{
public:
	struct BulletPathstr {
		
		uint8 OwnerTankIndex;
		XMVECTOR Dir;
		XMVECTOR Pos;
		
	};

public:
	CBulletPath();
	CBulletPath(CBulletPath& rhs);
	virtual ~CBulletPath() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

public:
	InstanceData CreateBulletTrailInstance(const XMVECTOR& oldPos, const XMVECTOR& newPos);

	bool CheckCollisionWithTerrain();

	bool CheckCollisionWithTank();

	uint8 OwnerTankIndex;

public:
	// Shot / hit effects shared by single player and the network handlers.
	// Spawn_Shot      : bullet trail + muzzle smoke + shot sound + muzzle flash light (what S_BULLET_ADD does)
	// Spawn_Explosion : explosion effect + sound + explosion light (what S_WEAPON_HIT does)
	static void Spawn_Shot(uint8 ownerTankIndex, _fvector vPos, _fvector vDir);
	static void Spawn_Explosion(_fvector vPos);
	static void Add_MuzzleFlashLight(_fvector vPos);	// point light only (used by the network handler too)
	static void Add_ExplosionLight(_fvector vPos);
private:
	CVIBuffer_Geos* m_VIBuffer;
	vector<InstanceData> BulletDatas;
	XMVECTOR m_Dir;
	class CTerrain* m_Terrain;
	float m_fAddBulletTime = 0.0016f;
	float m_fDeltaTime = 0.f;
	XMVECTOR m_Pos1;
	XMVECTOR m_Pos2;
	float m_fYSpeed = 0.f;
	bool isCollision = false;
	float m_fCollisionDeltatime = 0.f;

public:
	void Free() override;
	static CBulletPath* Create();
	CRenderObject* Clone(void* pArg);

};

END

