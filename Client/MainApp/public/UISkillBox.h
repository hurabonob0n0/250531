#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

#define  SkillboxScaleX 0.13f
#define  defaultScaleY 1.777f

BEGIN(Engine)
class CVIBuffer_Quad;
class CVIBuffer_Geos;
END

BEGIN(Client)

class CUISkillBox : public CUIObject
{
public:
	CUISkillBox();
	CUISkillBox(CUISkillBox& rhs);
	virtual ~CUISkillBox() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	CVIBuffer_Quad* m_VIBuffer;

private:
	int AIRSTRIKESKILL_BoxMat;
	int DRONESKILL_BoxMat;
	int FALLOWSKILL_BoxMat;
	int COLLTIME_BoxMat;

public:
	void Free() override;
	static CUISkillBox* Create();
	CUISkillBox* Clone(void* pArg);

};

END