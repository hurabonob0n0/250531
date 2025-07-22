#pragma once
#include "Client_Defines.h"
#include "RenderObject.h"

BEGIN(Engine)
class CVIBuffer_Geos;
class CBBinding;
END

BEGIN(Client)

class CWinningTeam : public CRenderObject
{
public:
	CWinningTeam();
	CWinningTeam(CWinningTeam& rhs);
	virtual ~CWinningTeam() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Geos* m_VIBuffer;
	CBBinding* m_CBBindingCom;

public:
	void Free() override;
	static CWinningTeam* Create();
	CRenderObject* Clone(void* pArg);

};

END