#pragma once
#include "Client_Defines.h"
#include "RenderObject.h"

BEGIN(Engine)
class CVIBuffer_Quad;
class CBBinding;
END

BEGIN(Client)

class CPing : public CRenderObject
{
public:
	/*struct PingDesc {
		_uint WhichCam;
	};*/

public:
	CPing();
	CPing(CPing& rhs);
	virtual ~CPing() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

private:
	void Set_Position();

public:
	void Set_Position(float x, float y, float z);
	_vector Get_Pos();

private:
	CVIBuffer_Quad* m_VIBuffer;
	CBBinding* m_CBBindingCom;
	class CTerrain* m_Terrain;
	CTransform* m_CameraFree;
	CTransform* m_DroneCamera;


	float m_TimeDelta = 0;
private:
	_uint m_MatIndex = 0;
	_vector LayContactPos;

public:
	void Free() override;
	static CPing* Create();
	CPing* Clone(void* pArg);

};

END