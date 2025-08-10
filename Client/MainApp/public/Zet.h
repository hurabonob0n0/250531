#pragma once
#include "Client_Defines.h"
#include "RenderObject.h"

BEGIN(Engine)
class CMeshModel;
class CBBinding;
END

BEGIN(Client)

class CZet : public CRenderObject
{
public:
	CZet();
	CZet(CZet& rhs);
	virtual ~CZet() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

public:
	void Set_StartPos_And_Move(_uint Num) {
		
		int num = (Num - 1) / 3 - 1;
		m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(-1000.f, 300.f , num * StartPos, 1.f));
		isAlive = true;
	}

private:
	float StartPos = 340.f;
	bool isAlive = false;
	_uint SelectNum = 0;

private:
	CMeshModel* m_VIBuffer;
	CBBinding* m_CBBindingCom;
	

public:
	void Free() override;
	static CZet* Create();
	CRenderObject* Clone(void* pArg);
};

END