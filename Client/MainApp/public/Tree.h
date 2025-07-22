#pragma once
#include "Client_Defines.h"
#include "RenderObject.h"

BEGIN(Engine)
class CMeshModel;
class CBBinding;
END

BEGIN(Client)

class CTree : public CRenderObject
{
public:
	CTree();
	CTree(CTree& rhs);
	virtual ~CTree() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CMeshModel* m_VIBuffer;
	CBBinding* m_CBBindingCom;
	CBBinding* m_CBBindingCom2;
	_uint		m_Test = 0;

public:
	void Free() override;
	static CTree* Create();
	CRenderObject* Clone(void* pArg);

};

END