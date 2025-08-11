#include "Client_pch.h"
#include "Drone.h"
#include "GameInstance.h"
#include "ClientPacketHandler.h"
#include "ServiceManager.h"
#include "Network_Manager.h"

CDrone::CDrone() : CRenderObject()
{
}

CDrone::CDrone(CDrone& rhs) : CRenderObject(rhs)
{
}

HRESULT CDrone::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CDrone::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_NONLIGHT;

    __super::Initialize(pArg);

    m_CBBindingCom = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);

    m_VIBuffer = (CMeshModel*)m_GameInstance->Get_Component("DroneModel");

    MaterialData MD{};

    XMStoreFloat4x4(&MD.MatTransform, XMMatrixIdentity());
    MD.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("DroneD", CTexture::Create(L"../bin/Models/Drone/Drone.dds"));
    MD.NormalMapIndex = 200;// CGameInstance::Get_Instance()->Add_Texture("DroneD", CTexture::Create(L"../bin/Models/Drone/Drone.dds"));
    m_CBBindingCom->Set_MaterialIndex(CGameInstance::Get_Instance()->Add_Material("DroneMat", MD));
    m_CBBindingCom->Set_TexCoordMatrix(XMMatrixIdentity());

    return S_OK;
}

void CDrone::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);

    //m_TransformCom->Go_Straight(fTimeDelta * 10.f);

    if (_myDrone) {
        if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE) {
            Update_Speed_and_Rot(fTimeDelta);
            Update_Rot_and_Pos(fTimeDelta);

            if (Network_Manager::GetInstance()->isConnected())
                SendMyPosToServer();
        }
    }

}

void CDrone::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);

    m_CBBindingCom->Set_World_TexCoord_And_Update(m_TransformCom, m_TexCoordTransformCom);
}

void CDrone::Render()
{
    m_CBBindingCom->Set_On_Shader();

    m_VIBuffer->Render(0);
}

void CDrone::Update_Speed_and_Rot(float fTimeDelta)
{
    if (m_GameInstance->Key_Pressing('D')) {
        m_fRightAxisSpeed += fTimeDelta * 30.f;
        if (m_fRightAxisSpeed >= m_fMaxSpeed)
            m_fRightAxisSpeed = m_fMaxSpeed;
    }
    if (m_GameInstance->Key_Pressing('A')) {
        m_fRightAxisSpeed -= fTimeDelta * 30.f;
        if (m_fRightAxisSpeed <= -m_fMaxSpeed)
            m_fRightAxisSpeed = -m_fMaxSpeed;
    }
    if (m_GameInstance->Key_Pressing('W')) {
        m_fLookAxisSpeed += fTimeDelta * 30.f;
        if (m_fLookAxisSpeed >= m_fMaxSpeed)
            m_fLookAxisSpeed = m_fMaxSpeed;
    }
    if (m_GameInstance->Key_Pressing('S')) {
        m_fLookAxisSpeed -= fTimeDelta * 30.f;
        if (m_fLookAxisSpeed <= -m_fMaxSpeed)
            m_fLookAxisSpeed = -m_fMaxSpeed;
    }
    if (m_GameInstance->Key_Pressing(VK_SPACE)) {
        m_fUpAxisSpeed += fTimeDelta * 30.f;
        if (m_fUpAxisSpeed >= m_fMaxSpeed)
            m_fUpAxisSpeed = m_fMaxSpeed;
    }
    if (m_GameInstance->Key_Pressing(VK_CONTROL)) {
        m_fUpAxisSpeed -= fTimeDelta * 30.f;
        if (m_fUpAxisSpeed <= -m_fMaxSpeed)
            m_fUpAxisSpeed = -m_fMaxSpeed;
    }

    if (!m_GameInstance->Key_Pressing('D') && !m_GameInstance->Key_Pressing('A'))
    {
        if (m_fRightAxisSpeed >= 0.5f)
            m_fRightAxisSpeed -= 15.f * fTimeDelta;
        else if (m_fRightAxisSpeed >= -0.5f && m_fRightAxisSpeed < 0.5f)
            m_fRightAxisSpeed = 0.f;
        else
            m_fRightAxisSpeed += 15.f * fTimeDelta;
    }

    if (!m_GameInstance->Key_Pressing('W') && !m_GameInstance->Key_Pressing('S'))
    {
        if (m_fLookAxisSpeed >= 0.5f)
            m_fLookAxisSpeed -= 15.f * fTimeDelta;
        else if (m_fLookAxisSpeed >= -0.5f && m_fLookAxisSpeed < 0.5f)
            m_fLookAxisSpeed = 0.f;
        else
            m_fLookAxisSpeed += 15.f * fTimeDelta;
    }

    if (!m_GameInstance->Key_Pressing(VK_SPACE) && !m_GameInstance->Key_Pressing(VK_CONTROL))
    {
        if (m_fUpAxisSpeed >= 0.5f)
            m_fUpAxisSpeed -= 15.f * fTimeDelta;
        else if (m_fUpAxisSpeed >= -0.5f && m_fUpAxisSpeed < 0.5f)
            m_fUpAxisSpeed = 0.f;
        else
            m_fUpAxisSpeed += 15.f * fTimeDelta;
    }

    m_fYawRot += m_GameInstance->Get_Mouse_XDelta() * 0.005f;

    m_fRollRot = XMConvertToRadians(-m_fRightAxisSpeed / m_fMaxSpeed * 45.f);

    m_fPitchRot = XMConvertToRadians(m_fLookAxisSpeed / m_fMaxSpeed * 45.f);
}

void CDrone::Update_Rot_and_Pos(float fTimeDelta)
{
    XMVECTOR RAxisDelta{ 1.f,0.f,0.f,0.f };
    XMVECTOR LAxisDelta{ 0.f,0.f,1.f,0.f };
    XMVECTOR UAxisDelta{ 0.f,1.f,0.f,0.f };
    RAxisDelta = XMVector4Transform(RAxisDelta, XMMatrixRotationY(m_fYawRot)) * fTimeDelta * m_fRightAxisSpeed;
    LAxisDelta = XMVector4Transform(LAxisDelta, XMMatrixRotationY(m_fYawRot)) * fTimeDelta * m_fLookAxisSpeed;
    UAxisDelta *= fTimeDelta * m_fUpAxisSpeed;

    m_vPos += RAxisDelta + LAxisDelta + UAxisDelta;

    XMMATRIX Rot = XMMatrixRotationRollPitchYaw(m_fPitchRot,m_fYawRot, m_fRollRot);
    
    m_TransformCom->Set_WorldMatrix(Rot);

    m_TransformCom->Set_State(CTransform::STATE_POSITION, m_vPos);
}

void CDrone::Free()
{
    Safe_Release(m_VIBuffer);
    Safe_Release(m_CBBindingCom);

    __super::Free();
}

CDrone* CDrone::Create()
{
    CDrone* pInstance = new CDrone;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CRenderObject* CDrone::Clone(void* pArg)
{
    CDrone* pInstance = new CDrone(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}

void CDrone::Set_My_DronePos_OnTank(XMFLOAT4X4& world)
{
    float posX = world._41;
    float posY = world._42 + 50.f; // Y°ª¸¸ +50
    float posZ = world._43;

    m_vPos = { posX, posY, posZ, 1.f };

}

void CDrone::SetOtherDroneMat(float PosX, float PosY,float PosZ, float Yaw, float Roll, float pitch)
{
    m_vPos = { PosX , PosY, PosZ, 1.f };
    m_fYawRot = Yaw;
    m_fRollRot = Roll;
    m_fPitchRot = pitch;
}

void CDrone::SendMyPosToServer()
{
    float x = XMVectorGetX(m_vPos);
    float y = XMVectorGetY(m_vPos);
    float z = XMVectorGetZ(m_vPos);
    auto sendBuffer = ClientPacketHandler::Make_C_DRONE_MOVE(x,y,z, m_fYawRot, m_fRollRot, m_fPitchRot);
    ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);

}

