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

