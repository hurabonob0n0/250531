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

	//void Set_Scale(float fScale) {
	//	_matrix mat = Get_WorldMatrix();
	//	mat.r[0] *= 2;
	//	mat.r[1] *= 2;
	//	mat.r[2] *= 2;

	//	XMStoreFloat4x4(&m_WorldMatrix,)
	//}

public:
	HRESULT Go_Straight(float fTimeDelta);
	HRESULT Go_Backward(float fTimeDelta);
	HRESULT Go_Left(float fTimeDelta);
	HRESULT Go_Right(float fTimeDelta);
	/*HRESULT Go_Direction(XMFLOAT3 fDir, float fTimeDelta);
	HRESULT Move_Dir_Length(XMFLOAT3 fDir, float fLength);*/

	void Turn(XMVECTOR vAxis, float fTimeDelta);
	void Rotation(XMVECTOR vAxis, float fRadian);

	/*void Look_At(_fvector vTargetPoint);
	void Chase_Target(_fvector vTargetPoint, _float fTimeDelta, _float fMargin = 0.1f);*/

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