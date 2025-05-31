#include "Client_pch.h"
#include "Tank.h"
#include "Client_Defines.h"
#include "GameInstance.h"
#include "ClientPacketHandler.h"
#include "ServiceManager.h"



CTank::CTank() : CRenderObject()
{
}

CTank::CTank(CTank& rhs) : CRenderObject(rhs)
{
}

HRESULT CTank::Initialize_Prototype()
{
    __super::Initialize_Prototype();

    return S_OK;
}

HRESULT CTank::Initialize(void* pArg)
{
    m_RG = CRenderer::RG_NONBLEND;

    __super::Initialize(pArg);

    m_VIBuffer = (CModel*)m_GameInstance->Get_Component("ModelCom");

    MaterialData mat{};
    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_Glacis_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glacis_Plate_Base_color.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_Glacis_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glacis_Plate_Normal.dds"));
    m_VIBuffer->Set_MatOffsets(mat.DiffuseMapIndex - 1); // Camera를 먼저 추가한다고 가정.
    m_GameInstance->Add_Material("Tank_Glacis", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_Glass_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glass_Base_color.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_Glass_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glass_Normal.dds"));
    m_GameInstance->Add_Material("Tank_Glass", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_Fence_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Fences_Base_color.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_Fence_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Fences_Normal.dds"));
    m_GameInstance->Add_Material("Tank_Fence", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_Wheels_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Wheels_Base_color.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_Wheels_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Wheels_Normal.dds"));
    m_GameInstance->Add_Material("Tank_Wheels", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_Turret_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Turret_Base_color.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_Turret_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Turret_Normal.dds"));
    m_GameInstance->Add_Material("Tank_Turret", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_W_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_W_Base_Base_color.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_W_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_W_Base_Normal.dds"));
    m_GameInstance->Add_Material("Tank_W", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_Sprocket_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Sprocket_Base_color.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_Sprocket_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Sprocket_Normal.dds"));
    m_GameInstance->Add_Material("Tank_Sprocket", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_Tracks_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Tracks_Base_color.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_Tracks_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Tracks_Normal.dds"));
    m_GameInstance->Add_Material("Tank_Tracks", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_M250_D", CTexture::Create(L"../bin/Models/TankDDS/M2-50_Base_color.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_M250_N", CTexture::Create(L"../bin/Models/TankDDS/M2-50_Normal.dds"));
    m_GameInstance->Add_Material("Tank_M250", mat);

    mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_M240P_D", CTexture::Create(L"../bin/Models/TankDDS/M240P_Base_color.dds"));
    mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_M240P_N", CTexture::Create(L"../bin/Models/TankDDS/M240P_Normal.dds"));
    m_GameInstance->Add_Material("Tank_M240P", mat);

	m_pPhysicsEngine = MyPhysicsEngine::CMyPhysicsEngine::Get_Instance();

    return S_OK;
}

void CTank::Tick(float fTimeDelta)
{
    //if (_myPlayer) {

    //    if (m_GameInstance->Key_Pressing(VK_UP)) {
    //        m_TransformCom->Go_Straight(fTimeDelta * 30.f);
    //        //_float4 tempPosition;
    //        //XMStoreFloat4(&tempPosition, m_TransformCom->Get_State(CTransform::STATE_POSITION));
    //        //SendBufferRef sendBuffer = ClientPacketHandler::Make_C_MOVE(tempPosition.x, tempPosition.y, tempPosition.z);
    //        //ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);
    //    }

    //    if (m_GameInstance->Key_Pressing(VK_LEFT)) {
    //        m_TransformCom->Go_Left(fTimeDelta * 30.f);
    //        //_float4 tempPosition;
    //        //XMStoreFloat4(&tempPosition, m_TransformCom->Get_State(CTransform::STATE_POSITION));
    //        //SendBufferRef sendBuffer = ClientPacketHandler::Make_C_MOVE(tempPosition.x, tempPosition.y, tempPosition.z);
    //        //ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);

    //    }

    //    if (m_GameInstance->Key_Pressing(VK_DOWN)) {
    //        m_TransformCom->Go_Backward(fTimeDelta * 30.f);
    //        //_float4 tempPosition;
    //        //XMStoreFloat4(&tempPosition, m_TransformCom->Get_State(CTransform::STATE_POSITION));
    //        //SendBufferRef sendBuffer = ClientPacketHandler::Make_C_MOVE(tempPosition.x, tempPosition.y, tempPosition.z);
    //        //ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);

    //    }

    //    if (m_GameInstance->Key_Pressing(VK_RIGHT)) {
    //        m_TransformCom->Go_Right(fTimeDelta * 30.f);
    //        //_float4 tempPosition;
    //        //XMStoreFloat4(&tempPosition, m_TransformCom->Get_State(CTransform::STATE_POSITION));
    //        //SendBufferRef sendBuffer = ClientPacketHandler::Make_C_MOVE(tempPosition.x, tempPosition.y, tempPosition.z);
    //        //ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);

    //    }

    //    _float4 Position;

    //    XMStoreFloat4(&Position, m_TransformCom->Get_State(CTransform::STATE_POSITION));

    //    float TerrainY = ((CVIBuffer_Terrain*)(m_GameInstance->GetPrototype("TerrainCom")))->Get_Terrain_Heights(Position.x, Position.z);

    //    m_TransformCom->Set_State(CTransform::STATE_POSITION, _vector{ Position.x,TerrainY - 8.f,Position.z,1.f });

    //}

    /*if (m_GameInstance->Key_Pressing(VK_RIGHT))
        m_TransformCom->Turn({ 0.f,1.f,0.f,0.f }, fTimeDelta);

    if (m_GameInstance->Key_Pressing(VK_LEFT))
        m_TransformCom->Turn({ 0.f,1.f,0.f,0.f }, -fTimeDelta);

    if (m_GameInstance->Key_Pressing(VK_UP))
        m_TransformCom->Turn(m_TransformCom->Get_State(CTransform::STATE_RIGHT), -fTimeDelta);

    if (m_GameInstance->Key_Pressing(VK_DOWN))
        m_TransformCom->Turn(m_TransformCom->Get_State(CTransform::STATE_RIGHT), fTimeDelta);

    if (m_GameInstance->Key_Pressing(VK_SPACE))
        m_TransformCom->Go_Straight(fTimeDelta);

    if (m_GameInstance->Key_Pressing(VK_CONTROL))
        m_TransformCom->Go_Backward(fTimeDelta);*/

    if (m_GameInstance->Key_Down('U'))
        m_TankConsrolState.leftThrust = true;

    if (m_GameInstance->Key_Down('J'))
        m_TankConsrolState.leftBrake = true;

    if (m_GameInstance->Key_Down('O'))
        m_TankConsrolState.rightThrust = true;

    if (m_GameInstance->Key_Down('L'))
        m_TankConsrolState.rightBrake = true;

    if (m_GameInstance->Key_Up('U'))
        m_TankConsrolState.leftThrust = false;

    if (m_GameInstance->Key_Up('J'))
        m_TankConsrolState.leftBrake = false;

    if (m_GameInstance->Key_Up('O'))
        m_TankConsrolState.rightThrust = false;

    if (m_GameInstance->Key_Up('L'))
        m_TankConsrolState.rightBrake = false;

    m_pPhysicsEngine->Set_Tank_ControlState(m_TankConsrolState);



    /*if (m_GameInstance->Key_Down(VK_SPACE))
    {
        if (m_CBIndex < 9)
            ++m_CBIndex;
        else
            m_CBIndex = 0;
    }

    _uint Matindex = m_MatIndicies[m_CBIndex];

    if (m_GameInstance->Key_Down(VK_RIGHT))
        if (Matindex < 8)
            m_MatIndicies[m_CBIndex] += 1;

    if (m_GameInstance->Key_Down(VK_LEFT))
        if (Matindex > 0)
            m_MatIndicies[m_CBIndex] -= 1;*/

}

void CTank::LateTick(float fTimeDelta)
{
    __super::LateTick(fTimeDelta);

 


    PxMat44 pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_CHASSIS);

    // PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
    XMMATRIX mat = XMMATRIX(
        XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
        XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
        XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
        XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
    );

    m_VIBuffer->Make_Root_Combined_Matrix(mat);//m_TransformCom->Get_WorldMatrix());

    m_VIBuffer->Invalidate_Bones();

    m_VIBuffer->Update();
}

void CTank::Render()
{
    for(int i = 0; i < 55; ++i)
        m_VIBuffer->Render(i);
}

void CTank::Free()
{
    __super::Free();
    Safe_Release(m_VIBuffer);
}

CTank* CTank::Create()
{
    CTank* pInstance = new CTank;
    pInstance->Initialize_Prototype();
    return pInstance;
}

CRenderObject* CTank::Clone(void* pArg)
{
    CTank* pInstance = new CTank(*this);
    pInstance->Initialize(pArg);
    return pInstance;
}
