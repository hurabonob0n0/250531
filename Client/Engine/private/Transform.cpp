#include "Transform.h"

CTransform::CTransform(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) : CComponent(pDevice, pCommandList)
{
}

CTransform::CTransform(const CTransform& rhs) : CComponent(rhs)
{
}

void CTransform::Adjust_Axis(XMVECTOR Look)
{
	_float3 scale = Get_Scaled();

	_vector		vRight = XMVectorSet(1.f,0.f,0.f,0.f);
	_vector		vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector     vLook = Look;

	vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));

	vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));


	vRight *= scale.x;
	vUp *= scale.y;
	vLook *= scale.z;

	Set_State(STATE_RIGHT, vRight);
	Set_State(STATE_UP, vUp);
	Set_State(STATE_LOOK, vLook);
}



HRESULT CTransform::Initialize_Prototype()
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
	return S_OK;
}

HRESULT CTransform::Initialize(void* pArg)
{
	//_matrix* matrix = (_matrix*)pArg;

	if (pArg == nullptr)
		XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
	else
		XMStoreFloat4x4(&m_WorldMatrix, *(_matrix*)pArg);

	return S_OK;
}


CTransform* CTransform::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
{
	CTransform* pInstance = new CTransform(pDevice, pCommandList);
	pInstance->Initialize_Prototype();
	return pInstance;
}

CComponent* CTransform::Clone(void* pArg)
{
	CComponent* pInstance = new CTransform(*this);
	pInstance->Initialize(pArg);
	return pInstance;
}

HRESULT CTransform::Go_Straight(float fTimeDelta)
{
	XMVECTOR	vPosition = Get_State(STATE_POSITION);
	XMVECTOR	vLook = Get_State(STATE_LOOK);

	vPosition += XMVector3Normalize(vLook) * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);

	return S_OK;
}

HRESULT CTransform::Go_Backward(float fTimeDelta)
{
	XMVECTOR	vPosition = Get_State(STATE_POSITION);
	XMVECTOR	vLook = Get_State(STATE_LOOK);

	vPosition -= XMVector3Normalize(vLook) * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);

	return S_OK;
}

HRESULT CTransform::Go_Left(float fTimeDelta)
{
	XMVECTOR	vPosition = Get_State(STATE_POSITION);
	XMVECTOR	vLook = Get_State(STATE_RIGHT);

	vPosition -= XMVector3Normalize(vLook) * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);

	return S_OK;
}

HRESULT CTransform::Go_Right(float fTimeDelta)
{
	XMVECTOR	vPosition = Get_State(STATE_POSITION);
	XMVECTOR	vLook = Get_State(STATE_RIGHT);

	vPosition += XMVector3Normalize(vLook) * fTimeDelta;

	Set_State(STATE_POSITION, vPosition);

	return S_OK;
}

void CTransform::Turn(XMVECTOR vAxis, float fRadian)
{
	_float3		vScaled = Get_Scaled();

	_vector		vRight = Get_State(STATE_RIGHT);
	_vector		vUp = Get_State(STATE_UP);
	_vector		vLook = Get_State(STATE_LOOK);

	_matrix		RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);

	// XMVector3TransformNormal();
	Set_State(STATE_RIGHT, XMVector4Transform(vRight, RotationMatrix));
	Set_State(STATE_UP, XMVector4Transform(vUp, RotationMatrix));
	Set_State(STATE_LOOK, XMVector4Transform(vLook, RotationMatrix));
}

void CTransform::Rotation(XMVECTOR vAxis, float fRadian)
{
	/*_float3 scale = Get_Scaled();

	XMFLOAT3 vRight;
	XMFLOAT3 vUp;
	XMVECTOR vLook = Get_State(STATE_LOOK);

	XMMATRIX		RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);

	XMVECTOR vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	vLook = XMVector3Normalize(XMVector3TransformNormal(vLook, RotationMatrix));

	XMStoreFloat3(&vRight, XMVector3Normalize(XMVector3Cross(vWorldUp, vLook)));

	XMStoreFloat3(&vUp, XMVector3Normalize(XMVector3Cross(vLook, XMLoadFloat3(&vRight))));

	Set_State(STATE_RIGHT, XMLoadFloat3(&vRight));
	Set_State(STATE_UP, XMLoadFloat3(&vUp));
	Set_State(STATE_LOOK, vLook);*/

	_float3 scale = Get_Scaled();

	_vector		vRight = Get_State(STATE_RIGHT);
	_vector		vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector     vLook = Get_State(STATE_LOOK);

	XMMATRIX		RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);

	vLook = XMVector3Normalize(XMVector3TransformNormal(vLook, RotationMatrix));

	vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));

	vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));


	vRight *= scale.x;
	vUp *= scale.y;
	vLook *= scale.z;

	Set_State(STATE_RIGHT, vRight);
	Set_State(STATE_UP, vUp);
	Set_State(STATE_LOOK, vLook);
}

void CTransform::Rotation1(XMVECTOR vAxis, float fRadian)
{
	_float3 scale = Get_Scaled();

	_vector		vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	_vector		vUp = Get_State(STATE_UP);
	_vector     vLook = XMVectorSet(0.f,0.f,1.f,0.f);

	XMMATRIX		RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);

	vUp = XMVector3Normalize(XMVector3TransformNormal(vUp, RotationMatrix));

	vLook = XMVector3Normalize(XMVector3Cross(vRight, vUp));

	vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));


	vRight *= scale.x;
	vUp *= scale.y;
	vLook *= scale.z;

	Set_State(STATE_RIGHT, vRight);
	Set_State(STATE_UP, vUp);
	Set_State(STATE_LOOK, vLook);

}

void CTransform::Orbit_For_TPS(_fvector vCenter, float fYRot, float fXRot)
{
	_vector vPosition = Get_State(STATE_POSITION);
	_vector vToCenter = vPosition - vCenter;

	XMMATRIX matRotationY = XMMatrixRotationY(fYRot);
	XMMATRIX matRotationX = XMMatrixRotationAxis(Get_State(STATE_RIGHT), fXRot);

	XMMATRIX matRot = matRotationX * matRotationY;

	vToCenter = XMVector3TransformNormal(vToCenter, matRot);

	// 4. 새 위치 = 중심 + 회전된 벡터
	_vector vNewPos = vCenter + vToCenter;

	Set_State(STATE_POSITION, vNewPos);

	// 5. 새 위치에서 중심을 바라보도록 방향 설정
	Look_At(vCenter);
}

void CTransform::Orbit_For_FPS(_fvector vCenter, float fYRot, float fXRot)
{
	_vector vPosition = Get_State(STATE_POSITION);
	_vector vToCenter = vCenter - vPosition;

	XMMATRIX matRotationY = XMMatrixRotationY(fYRot);
	XMMATRIX matRotationX = XMMatrixRotationAxis(Get_State(STATE_RIGHT), fXRot);

	XMMATRIX matRot = matRotationX * matRotationY;

	vToCenter = XMVector3TransformNormal(vToCenter, matRot);

	// 4. 새 위치 = 중심 + 회전된 벡터
	_vector vNewPos = vCenter + vToCenter;

	Set_State(STATE_POSITION, vNewPos);

	// 5. 새 위치에서 중심을 바라보도록 방향 설정
	Look_At(vNewPos + vToCenter);
}

void CTransform::Orbit(_fvector vCenter, float fYawDeg, float fPitchDeg, float fDistance, float fDeltaTime, float fLagSpeed)
{
	// 1. 회전 행렬 생성
	XMMATRIX matRotY = XMMatrixRotationY(XMConvertToRadians(fYawDeg));
	XMMATRIX matRotX = XMMatrixRotationAxis(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(fPitchDeg));
	XMMATRIX matRot = matRotX * matRotY;

	// 2. 회전된 거리 벡터 계산
	_vector vOffset = XMVectorSet(0.f, 0.f, -fDistance, 0.f);
	vOffset = XMVector3TransformNormal(vOffset, matRot);

	// 3. 목표 위치 계산
	_vector vGoalPos = vCenter + vOffset;

	// 4. 현재 위치 가져오기
	_vector vCurrPos = Get_State(STATE_POSITION);

	// 5. 보간
	float alpha = 1.f - expf(-fLagSpeed * fDeltaTime);
	_vector vNewPos = XMVectorLerp(vCurrPos, vGoalPos, alpha);

	// 6. 적용
	Set_State(STATE_POSITION, vNewPos);
	Look_At(vCenter);
}

void CTransform::Look_At(_fvector vTargetPoint)
{
	XMFLOAT3 vRight;
	XMFLOAT3 vUp;
	XMVECTOR vLook = vTargetPoint - Get_State(STATE_POSITION);

	float length = XMVectorGetX(XMVector3Length(vLook));

	XMVECTOR vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	vLook = XMVector3Normalize(vLook);

	XMStoreFloat3(&vRight, XMVector3Normalize(XMVector3Cross(vWorldUp, vLook)));

	XMStoreFloat3(&vUp, XMVector3Normalize(XMVector3Cross(vLook, XMLoadFloat3(&vRight))));

	Set_State(STATE_RIGHT, XMLoadFloat3(&vRight));
	Set_State(STATE_UP, XMLoadFloat3(&vUp));
	Set_State(STATE_LOOK, vLook);
}

void CTransform::Free()
{
	__super::Free();
}

