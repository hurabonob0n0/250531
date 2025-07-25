#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CTransform : public CComponent
{
public:
	enum STATE { STATE_RIGHT, STATE_UP, STATE_LOOK, STATE_POSITION, STATE_END };

public:
	CTransform(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	CTransform(const CTransform& rhs);
	virtual ~CTransform() = default;

public:

	void Identity() {
		XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
	}

	void Adjust_Axis(XMVECTOR Look);

	_float3 Get_Scaled() const {
		return _float3(XMVectorGetX(XMVector3Length(Get_State(STATE_RIGHT))),
			XMVectorGetX(XMVector3Length(Get_State(STATE_UP))),
			XMVectorGetX(XMVector3Length(Get_State(STATE_LOOK))));
	}

	XMVECTOR Get_State(STATE eState) const {
		return XMLoadFloat4x4(&m_WorldMatrix).r[eState];
	}

	XMFLOAT4X4 Get_WorldFloat4x4_Inverse() {
		XMFLOAT4X4	WorldMatrixInverse;
		XMStoreFloat4x4(&WorldMatrixInverse, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));
		return WorldMatrixInverse;
	}

	XMMATRIX Get_WorldMatrix() const {
		return XMLoadFloat4x4(&m_WorldMatrix);
	}

	XMMATRIX Get_WorldMatrix_Inverse() {
		return XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix));
	}

	void Set_State(STATE eState, XMVECTOR vState)
	{
		XMMATRIX TransformMatrix = XMLoadFloat4x4(&m_WorldMatrix);
		TransformMatrix.r[eState] = vState;
		XMStoreFloat4x4(&m_WorldMatrix, TransformMatrix);
	}

	void Set_WorldMatrix(const XMFLOAT4X4& Worldmat)
	{
		m_WorldMatrix = Worldmat;
	}

	void Set_WorldMatrix(const XMMATRIX& Worldmat)
	{
		XMStoreFloat4x4(&m_WorldMatrix, Worldmat);
	}

	void Set_Scale(float fScale) {
		_matrix mat = Get_WorldMatrix();
		mat.r[0] *= fScale;
		mat.r[1] *= fScale;
		mat.r[2] *= fScale;

		XMStoreFloat4x4(&m_WorldMatrix, mat);
	}

	void Set_Scale(STATE state,float fScale) {
		_matrix mat = Get_WorldMatrix();
		mat.r[state] *= fScale;

		XMStoreFloat4x4(&m_WorldMatrix, mat);
	}

public:
	HRESULT Go_Straight(float fTimeDelta);
	HRESULT Go_Backward(float fTimeDelta);
	HRESULT Go_Left(float fTimeDelta);
	HRESULT Go_Right(float fTimeDelta);
	/*HRESULT Go_Direction(XMFLOAT3 fDir, float fTimeDelta);
	HRESULT Move_Dir_Length(XMFLOAT3 fDir, float fLength);*/

	void Turn(XMVECTOR vAxis, float fRadian);
	void Rotation(XMVECTOR vAxis, float fRadian);
	void Rotation1(XMVECTOR vAxis, float fRadian);
	void Orbit_For_TPS(_fvector vCenter, float fYRot, float fXRot);
	void Orbit_For_FPS(_fvector vCenter, float fYRot, float fXRot);
	void Orbit(_fvector vCenter, float fYawDeg, float fPitchDeg, float fDistance, float fDeltaTime, float fLagSpeed = 6.f);

	/*void Look_At(_fvector vTargetPoint);
	void Chase_Target(_fvector vTargetPoint, _float fTimeDelta, _float fMargin = 0.1f);*/

	void Look_At(_fvector vTargetPoint);

public:
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize(void* pArg) override;

private:
	XMFLOAT4X4	m_WorldMatrix;

public:
	static CTransform* Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
	CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END