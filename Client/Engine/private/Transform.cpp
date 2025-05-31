#include "Transform.h"

CTransform::CTransform(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) : CComponent(pDevice,pCommandList)
{
}

CTransform::CTransform(const CTransform& rhs) : CComponent(rhs)
{
}


HRESULT CTransform::Initialize_Prototype()
{
    XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
    return S_OK;
}

HRESULT CTransform::Initialize(void* pArg)
{
    //_matrix* matrix = (_matrix*)pArg;

    if(pArg == nullptr)
        XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
    else
        XMStoreFloat4x4(&m_WorldMatrix, *(_matrix*)pArg);

    return S_OK;
}


CTransform* CTransform::Create(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
{
    CTransform* pInstance = new CTransform(pDevice,pCommandList);
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

void CTransform::Turn(XMVECTOR vAxis, float fTimeDelta)
{
    XMFLOAT3 vRight;
    XMFLOAT3 vUp;
    XMVECTOR vLook = Get_State(STATE_LOOK);

    XMMATRIX		RotationMatrix = XMMatrixRotationAxis(vAxis, fTimeDelta);

    XMVECTOR vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    vLook = XMVector3Normalize(XMVector3TransformNormal(vLook, RotationMatrix));

    XMStoreFloat3(&vRight, XMVector3Normalize(XMVector3Cross(vWorldUp, vLook)));

    XMStoreFloat3(&vUp, XMVector3Normalize(XMVector3Cross(vLook, XMLoadFloat3(&vRight))));

    Set_State(STATE_RIGHT, XMLoadFloat3(&vRight));
    Set_State(STATE_UP, XMLoadFloat3(&vUp));
    Set_State(STATE_LOOK, vLook);
}

void CTransform::Rotation(XMVECTOR vAxis, float fRadian)
{
}

void CTransform::Free()
{
    __super::Free();
}

