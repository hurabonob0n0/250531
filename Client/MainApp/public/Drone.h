#pragma once
#include "Client_Defines.h"
#include "RenderObject.h"

BEGIN(Engine)
class CMeshModel;
class CBBinding;
END


namespace FMOD { class Channel; }
class FMOD_Manager;

BEGIN(Client)

class CDrone : public CRenderObject
{
public:
	CDrone();
	CDrone(CDrone& rhs);
	virtual ~CDrone() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(float fTimeDelta);
	virtual void LateTick(float fTimeDelta);
	virtual void Render();

public:

	float	Get_DroneYaw() {
		return m_fYawRot;
	};
	void	Set_DroneYaw(float Yaw) {
		m_fYawRot = Yaw;
	};


	float	Get_DroneRoll() {
		return m_fRollRot;
	};
	void	Set_DroneRoll(float Roll) {
		m_fRollRot = Roll;
	};


	float	Get_DronePitch() {
		return m_fPitchRot;
	};
	void	Set_DronePitch(float Pitch) {
		m_fPitchRot = Pitch;
	};
private:
	void Update_Speed_and_Rot(float fTimeDelta);
	void Update_Rot_and_Pos(float fTimeDelta);


private:
	CMeshModel* m_VIBuffer;
	CBBinding* m_CBBindingCom;
	float m_fMaxSpeed = 25.f;
	float m_fRightAxisSpeed = 0.f;
	float m_fLookAxisSpeed = 0.f;
	float m_fUpAxisSpeed = 0.f;
	float m_fYawRot = 0.f;
	float m_fRollRot = 0.f;
	float m_fPitchRot = 0.f;
	XMVECTOR m_vPos{ -0.f,500.f,-0.f,1.f };

public:
	void Free() override;
	static CDrone* Create();
	CRenderObject* Clone(void* pArg);

public:

	bool _myDrone = false;
	void Set_My_Drone() { _myDrone = true; };
	void Set_My_DronePos_OnTank(XMFLOAT4X4& world);


	void SetOtherDroneMat(float PosX, float PosY, float PosZ, float Yaw, float Roll, float pitch);
	void SendMyPosToServer();

	void Update_Follow_Tank(float dt);


	FMOD::Channel* m_pFlyChannel = nullptr;

public:

	bool m_followTank = false;   // Z로 토글
	float m_followHeight = 30.f; // 탱크 위 고정 높이
};

END