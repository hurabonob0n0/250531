#include "Client_pch.h"
#include "Drone.h"
#include "GameInstance.h"
#include "ClientPacketHandler.h"
#include "Network_Manager.h"
#include "FMOD_Manager.h"
#include "Tank.h"

CDrone::CDrone() : CRenderObject()
{
    _myDrone = false;
}

CDrone::CDrone(CDrone& rhs) : CRenderObject(rhs)
{
    _myDrone = false;
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

    

    AudioVec3 startPos{ m_vPos.m128_f32[0], m_vPos.m128_f32[1], m_vPos.m128_f32[2] };
    AudioVec3 vel{ 0,0,0 };
    FMOD_Manager::Get_Instance()->Play3D_ReturnChannel("Drone_Fly_Sound", startPos, vel, &m_pFlyChannel, 1.0f, false);

 

    return S_OK;
}

void CDrone::Tick(float fTimeDelta)
{
    __super::Tick(fTimeDelta);

    //m_TransformCom->Go_Straight(fTimeDelta * 10.f);

    if (Network_Manager::GetInstance()->isConnected()) {

        if (_myDrone && Network_Manager::GetInstance()->MyPosMode != POS_DRIVER) {

            if (m_followTank) {
                Update_Follow_Tank(fTimeDelta);
            }

            if (Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE) {

                if (m_GameInstance->Key_Down('G')) {
                    m_followTank = !m_followTank;
                }

                if (m_followTank) {
                    m_fRollRot = m_fPitchRot = m_fYawRot =m_fLookAxisSpeed = m_fRightAxisSpeed = m_fUpAxisSpeed = 0.f;
                }
                else {
                    Update_Speed_and_Rot(fTimeDelta);
                    Update_Rot_and_Pos(fTimeDelta);
                }

               
            } 

            SendMyPosToServer();
        }
        else {
            Update_Rot_and_Pos(fTimeDelta);
        }
    }
    else {

        if (m_followTank) {
            Update_Follow_Tank(fTimeDelta);
        }


        if (_myDrone && Network_Manager::GetInstance()->MyControlTarget == CONTROL_DRONE) {

            if (m_GameInstance->Key_Down('G')) {
                m_followTank = !m_followTank;
            }


            if (m_followTank) {
                m_fRollRot = m_fPitchRot = m_fYawRot = m_fLookAxisSpeed = m_fRightAxisSpeed = m_fUpAxisSpeed = 0.f;
            }
            else {
                Update_Speed_and_Rot(fTimeDelta);
                Update_Rot_and_Pos(fTimeDelta);
            }


        }

    }

   

    if (m_pFlyChannel) {
        AudioVec3 pos{
            XMVectorGetX(m_vPos),
            XMVectorGetY(m_vPos),
            XMVectorGetZ(m_vPos)
        };
        AudioVec3 vel{ 0,0,0 }; // 필요시 속도 계산해서 전달
        m_pFlyChannel->set3DAttributes((FMOD_VECTOR*)&pos, (FMOD_VECTOR*)&vel);
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
        // 드론 높이가 최소 40.f 이상일 때만 하강 허용
        if (XMVectorGetY(m_vPos) > 50.f) {
            m_fUpAxisSpeed -= fTimeDelta * 30.f;
            if (m_fUpAxisSpeed <= -m_fMaxSpeed)
                m_fUpAxisSpeed = -m_fMaxSpeed;
        }
        else {
            // 최소 높이에 걸리면 하강 속도 0으로 고정
            m_fUpAxisSpeed = 0.f;
        }
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

    if (m_pFlyChannel) {
        m_pFlyChannel->stop();
        m_pFlyChannel = nullptr;
    }

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
    float posY = world._42 + 50.f; // Y값만 +50
    float posZ = world._43;

    m_vPos = { posX, posY, posZ, 1.f };

    m_TransformCom->Set_State(CTransform::STATE_POSITION, m_vPos);


}

void CDrone::SetOtherDroneMat(float PosX, float PosY,float PosZ, float Yaw, float Roll, float pitch)
{
    if (!_myDrone||Network_Manager::GetInstance()->MyPosMode == POS_DRIVER) {
        m_vPos = { PosX , PosY, PosZ, 1.f };
        m_fYawRot = Yaw;
        m_fRollRot = Roll;
        m_fPitchRot = pitch;
    }
}

void CDrone::SendMyPosToServer()
{
    float x = XMVectorGetX(m_vPos);
    float y = XMVectorGetY(m_vPos);
    float z = XMVectorGetZ(m_vPos);
    auto sendBuffer = ClientPacketHandler::Make_C_DRONE_MOVE(x,y,z, m_fYawRot, m_fRollRot, m_fPitchRot);
    Network_Manager::GetInstance()->Send(sendBuffer);

}

void CDrone::Update_Follow_Tank(float dt)
{
    // 내 탱크 가져오기
    CTank* myTank = nullptr;
    if (Network_Manager::GetInstance()->isConnected())
        myTank = static_cast<CTank*>(CGameInstance::Get_Instance()->GetGameObject("Tank", Network_Manager::GetInstance()->GetMyTankIndex()));
    else
        myTank = static_cast<CTank*>(CGameInstance::Get_Instance()->GetGameObject("Tank", 0));
    if (!myTank) return;

    // 2) 탱크 위치
    const DirectX::XMMATRIX tankWorld = myTank->Get_WorldMatrix();
    DirectX::XMFLOAT4X4 m; DirectX::XMStoreFloat4x4(&m, tankWorld);
    DirectX::XMVECTOR tankPos = DirectX::XMVectorSet(m._41, m._42, m._43, 1.f);

    // 3) 목표 위치(위로 m_followHeight)
    const DirectX::XMVECTOR targetPos = DirectX::XMVectorAdd(tankPos, DirectX::XMVectorSet(0.f, m_followHeight, 0.f, 0.f));

    {
        // 목표까지의 벡터/거리
        DirectX::XMVECTOR delta = DirectX::XMVectorSubtract(targetPos, m_vPos);
        float dist = DirectX::XMVectorGetX(DirectX::XMVector3Length(delta));

        if (dist > 1e-4f) {
            DirectX::XMVECTOR dir = DirectX::XMVector3Normalize(delta);

            const float maxSpeed = 15.f;     // ★ 초당 이동 속도(원하는 값으로 조절)
            float step = maxSpeed * dt;      // 이번 프레임 이동량

            if (step > dist) step = dist;    // 목표 근처에서 overshoot 방지

            m_vPos = DirectX::XMVectorAdd(m_vPos, DirectX::XMVectorScale(dir, step));
        }
    }

    // 5) (옵션) 회전 유지/갱신이 필요하면 여기서 처리

    // 6) 트랜스폼 반영
    const DirectX::XMMATRIX rot =
        DirectX::XMMatrixRotationRollPitchYaw(m_fPitchRot, m_fYawRot, m_fRollRot);
    m_TransformCom->Set_WorldMatrix(rot);
    m_TransformCom->Set_State(CTransform::STATE_POSITION, m_vPos);
}
