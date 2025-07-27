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

void CTank::Set_Team(int Team)
{
	m_VIBuffer->Set_Team(Team);
}

HRESULT CTank::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	matindex = CGameInstance::Get_Instance()->Get_Mat_Size();

	MaterialData mat{};
	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Glacis_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glacis_Plate_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Glacis_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glacis_Plate_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Glacis", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Glass_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glass_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Glass_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glass_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Glass", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Fence_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Fences_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Fence_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Fences_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Fence", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Wheels_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Wheels_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Wheels_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Wheels_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Wheels", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Turret_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Turret_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Turret_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Turret_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Turret", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_W_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_W_Base_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_W_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_W_Base_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_W", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Sprocket_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Sprocket_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Sprocket_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Sprocket_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Sprocket", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Tracks_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Tracks_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_Tracks_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Tracks_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_Tracks", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_M250_D", CTexture::Create(L"../bin/Models/TankDDS/M2-50_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_M250_N", CTexture::Create(L"../bin/Models/TankDDS/M2-50_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_M250", mat);

	mat.DiffuseMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_M240P_D", CTexture::Create(L"../bin/Models/TankDDS/M240P_Base_color.dds"));
	mat.NormalMapIndex = CGameInstance::Get_Instance()->Add_Texture("Tank_M240P_N", CTexture::Create(L"../bin/Models/TankDDS/M240P_Normal.dds"));
	CGameInstance::Get_Instance()->Add_Material("Tank_M240P", mat);

	return S_OK;
}

HRESULT CTank::Initialize(void* pArg)
{
	m_RG = CRenderer::RG_NONBLEND;

	__super::Initialize(pArg);

	//CModel* pModel = (CModel*)m_GameInstance->Get_Component("ModelCom");
	m_VIBuffer = (CModel*)m_GameInstance->Get_Component("ModelCom");

	m_VIBuffer->Set_MatOffsets(matindex);

	m_pPhysicsEngine = MyPhysicsEngine::CMyPhysicsEngine::Get_Instance();

	Set_Team(1);

	Initialize_For_PosinQuad();

	return S_OK;
}

void CTank::Initialize_For_PosinQuad()
{
	m_CBBindingQuad = (CBBinding*)m_GameInstance->Get_Component("CBBindingCom", nullptr);
	MaterialData mat{};

	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Posin", CTexture::Create(L"../bin/Models/Posin/Posin.dds"));

	m_CBBindingQuad->Set_MaterialIndex(m_GameInstance->Add_Material("PosinMat", mat));

	m_VIBufferQuad = (CVIBuffer_Quad*)m_GameInstance->Get_Component("VIBuffer_QuadCom");

	m_QuadWorldTransform = (CTransform*)m_GameInstance->Get_Component("TransformCom", nullptr);

	m_QuadTexTransform = (CTransform*)m_GameInstance->Get_Component("TransformCom", nullptr);
}

void CTank::Tick(float fTimeDelta)
{
	if (_myPlayer)
	{

		/*PopAllBulletMatrix([](const _matrix& mat) {
			_matrix tempMat = mat;
			CGameInstance::Get_Instance()->AddObject("DefaultObject", "BulletObj", &tempMat);
			});*/

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

		if (m_GameInstance->Mouse_Down(0)) {
			//m_GameInstance->AddObject("DefaultObject", "BoxObj", &ShotMatrix);
			//SendShootDataToServer();
		}

		m_pPhysicsEngine->Set_Tank_ControlState(m_TankConsrolState);
	}

	else
	{

		if (m_GameInstance->Key_Pressing('W'))
			m_TransformCom->Go_Straight(fTimeDelta * 10.f);

		if (m_GameInstance->Key_Pressing('A'))
			m_TransformCom->Go_Left(fTimeDelta * 10.f);

		if (m_GameInstance->Key_Pressing('S'))
			m_TransformCom->Go_Backward(fTimeDelta * 10.f);

		if (m_GameInstance->Key_Pressing('D'))
			m_TransformCom->Go_Right(fTimeDelta * 10.f);

		//if (m_GameInstance->Key_Pressing(VK_SPACE))
		//	m_TransformCom->Go_Right(fTimeDelta * 10.f);

		//if (m_GameInstance->Key_Pressing(VK_LCONTROL))
		//	m_TransformCom->Go_Right(fTimeDelta * 10.f);


	}


}

void CTank::LateTick(float fTimeDelta)
{
	__super::LateTick(fTimeDelta);

	RotPotap_And_Posin(fTimeDelta);

	if (_myPlayer)
	{
		PxMat44 pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_CHASSIS);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);

		_float4x4 matworld;
		XMStoreFloat4x4(&matworld, mat);

		m_TransformCom->Set_WorldMatrix(matworld);

#pragma region Wheel

		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_FIRST_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L1Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_SECOND_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R1Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_THIRD_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L2Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_FOURTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R2Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_FIFTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L3Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_SIXTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R3Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_SEVENTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L4Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_FIRST_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R4Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_SECOND_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L5Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_THIRD_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R5Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_FOURTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L6Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_FIFTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R6Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_SIXTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L7Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);

		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_SEVENTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R7Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y + 0.15f, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y + 0.15f, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y + 0.15f, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y + 0.15f, pxMat.column3.z, pxMat.column3.w)
		);
#pragma endregion


		_vector forward = m_TransformCom->Get_State(CTransform::STATE_LOOK);
		float yaw = atan2(XMVectorGetX(forward), XMVectorGetZ(forward));
		//float yawDeg = XMConvertToDegrees(yaw);

		XMMATRIX matPotap = XMMatrixRotationY(XMConvertToRadians(m_fPotapRotation - XMConvertToDegrees(yaw)));

		XMMATRIX matPosin = XMMatrixRotationY(XMConvertToRadians(m_fPosinRotation));

		//if(m_fPosinRotation  )

		/*XMMATRIX matPosinforDir = XMMatrixRotationX(-m_fPosinRotation);

		_vector worldforward = XMVectorSet(0, 0, 1, 0);
		worldforward = XMVector4Transform(worldforward, m_TransformCom->Get_WorldMatrix());
		worldforward = XMVector4Transform(worldforward, matPotap);
		worldforward = XMVector4Transform(worldforward, matPosinforDir);*/

		/*XMFLOAT3 vRight;
		XMFLOAT3 vUp;*/




		m_VIBuffer->Set_Transform_Matrix(0, mat); // Chassis
		m_VIBuffer->Set_Transform_Matrix(1, matPotap); // Potap
		//m_VIBuffer->Set_Transform_Matrix(2, matPosin);


		m_VIBuffer->Set_Transform_Matrix(24 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L1Mat); // Left First Wheel
		m_VIBuffer->Set_Transform_Matrix(26 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L2Mat); // Left Second Wheel
		m_VIBuffer->Set_Transform_Matrix(28 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L3Mat); // Left Third Wheel
		m_VIBuffer->Set_Transform_Matrix(30 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L4Mat); // Left Fourth Wheel
		m_VIBuffer->Set_Transform_Matrix(32 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L5Mat); // Left Fifth Wheel
		m_VIBuffer->Set_Transform_Matrix(34 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L6Mat); // Left Sixth Wheel
		m_VIBuffer->Set_Transform_Matrix(36 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * L7Mat); // Left Seventh Wheel
		m_VIBuffer->Set_Transform_Matrix(46 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R1Mat); // Right First Wheel
		m_VIBuffer->Set_Transform_Matrix(37 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R2Mat); // Right Second Wheel
		m_VIBuffer->Set_Transform_Matrix(35 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R3Mat); // Right Third Wheel
		m_VIBuffer->Set_Transform_Matrix(33 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R4Mat); // Right Fourth Wheel
		m_VIBuffer->Set_Transform_Matrix(44 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R5Mat); // Right Fifth Wheel
		m_VIBuffer->Set_Transform_Matrix(48 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R6Mat); // Right Sixth Wheel
		m_VIBuffer->Set_Transform_Matrix(42 + 3, XMMatrixRotationZ(XMConvertToRadians(90.f)) * R7Mat); // Right Seventh Wheel


		//   m_VIBuffer->Set_Transform_Matrix(0, m_TransformCom->Get_WorldMatrix()); // Chassis
		   //m_VIBuffer->Set_Transform_Matrix(1, m_TransformCom->Get_WorldMatrix()); // Potap

		m_VIBuffer->Invalidate_Bones();

		m_VIBuffer->Multiply_Mesh_Combined_Matrix(50, matPosin);
		m_VIBuffer->Multiply_Mesh_Combined_Matrix(51, matPosin);
		m_VIBuffer->Multiply_Mesh_Combined_Matrix(29, matPosin);

		m_VIBuffer->Update();

		//for server
		//SendMyStateToServer();

	}

	else
	{


		/*XMMATRIX matPotap = XMMatrixRotationY(m_fPotapRotation);

		XMMATRIX matPosin = XMMatrixRotationY(m_fPosinRotation);*/

		_vector forward = m_TransformCom->Get_State(CTransform::STATE_LOOK);
		float yaw = atan2(XMVectorGetX(forward), XMVectorGetZ(forward));
		//float yawDeg = XMConvertToDegrees(yaw);

		XMMATRIX matPotap = XMMatrixRotationY(XMConvertToRadians(m_fPotapRotation - XMConvertToDegrees(yaw)));

		XMMATRIX matPosin = XMMatrixRotationY(XMConvertToRadians(m_fPosinRotation));


		m_VIBuffer->Set_Transform_Matrix(0, m_TransformCom->Get_WorldMatrix()); // Chassis
		m_VIBuffer->Set_Transform_Matrix(1, matPotap); // Potap
		//m_VIBuffer->Set_Transform_Matrix(2, matPosin); // Posin

		m_VIBuffer->Multiply_Transform_Matrix(26 + 3, m_TransformCom->Get_WorldMatrix()); // Left Second Wheel
		m_VIBuffer->Multiply_Transform_Matrix(28 + 3, m_TransformCom->Get_WorldMatrix()); // Left Third Wheel
		m_VIBuffer->Multiply_Transform_Matrix(24 + 3, m_TransformCom->Get_WorldMatrix()); // Left First Wheel
		m_VIBuffer->Multiply_Transform_Matrix(30 + 3, m_TransformCom->Get_WorldMatrix()); // Left Fourth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(32 + 3, m_TransformCom->Get_WorldMatrix()); // Left Fifth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(34 + 3, m_TransformCom->Get_WorldMatrix()); // Left Sixth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(36 + 3, m_TransformCom->Get_WorldMatrix()); // Left Seventh Wheel
		m_VIBuffer->Multiply_Transform_Matrix(46 + 3, m_TransformCom->Get_WorldMatrix()); // Right First Wheel
		m_VIBuffer->Multiply_Transform_Matrix(37 + 3, m_TransformCom->Get_WorldMatrix()); // Right Second Wheel
		m_VIBuffer->Multiply_Transform_Matrix(35 + 3, m_TransformCom->Get_WorldMatrix()); // Right Third Wheel
		m_VIBuffer->Multiply_Transform_Matrix(33 + 3, m_TransformCom->Get_WorldMatrix()); // Right Fourth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(44 + 3, m_TransformCom->Get_WorldMatrix()); // Right Fifth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(48 + 3, m_TransformCom->Get_WorldMatrix()); // Right Sixth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(42 + 3, m_TransformCom->Get_WorldMatrix()); // Right Seventh Wheel

		//여기서 받은 데이터로 매트릭스 바꿔줌
		m_VIBuffer->Invalidate_Bones();

		m_VIBuffer->Multiply_Transform_Matrix(26 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Left Second Wheel
		m_VIBuffer->Multiply_Transform_Matrix(28 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Left Third Wheel
		m_VIBuffer->Multiply_Transform_Matrix(24 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Left First Wheel
		m_VIBuffer->Multiply_Transform_Matrix(30 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Left Fourth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(32 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Left Fifth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(34 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Left Sixth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(36 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Left Seventh Wheel
		m_VIBuffer->Multiply_Transform_Matrix(46 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Right First Wheel
		m_VIBuffer->Multiply_Transform_Matrix(37 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Right Second Wheel
		m_VIBuffer->Multiply_Transform_Matrix(35 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Right Third Wheel
		m_VIBuffer->Multiply_Transform_Matrix(33 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Right Fourth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(44 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Right Fifth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(48 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Right Sixth Wheel
		m_VIBuffer->Multiply_Transform_Matrix(42 + 3, m_TransformCom->Get_WorldMatrix_Inverse()); // Right Seventh Wheel

		
		m_VIBuffer->Multiply_Mesh_Combined_Matrix(50, matPosin);
		m_VIBuffer->Multiply_Mesh_Combined_Matrix(51, matPosin);
		m_VIBuffer->Multiply_Mesh_Combined_Matrix(29, matPosin);

		m_VIBuffer->Update();
	}


	Tick_For_Posin_Image(fTimeDelta);

	if (m_GameInstance->Mouse_Down(1))
		m_isFPS = !m_isFPS;
}

void CTank::Render()
{
	/*if (!m_isQuadTurn)
	{
		for (int i = 0; i < 55; ++i)
			m_VIBuffer->Render(i);
		m_isQuadTurn = true;
	}

	else if (_myPlayer)
	{
		if (m_isFPS) {
			Render_For_Posin_Image();
		}

		m_isQuadTurn = false;
	}

	else if (!_myPlayer)
		m_isQuadTurn = false;*/

	if (_myPlayer)
	{
		if (m_isFPS) {
			Render_For_Posin_Image();
		}
		else
		{
			for (int i = 0; i < 55; ++i)
				m_VIBuffer->Render(i);
		}
	}
	else
	{
		for (int i = 0; i < 55; ++i)
			m_VIBuffer->Render(i);
	}
}

void CTank::Set_PotapRotation(float fDegree)
{
	m_fCamPotapRot = fDegree;
}

void CTank::Set_PoSinpRotation(float fDegree)
{
	m_fCamPosinRot = fDegree;
}

void CTank::Set_ShotDir(XMVECTOR Vec)
{
	vShotDir = Vec;
}

void CTank::Set_ShotMatrix(_matrix mat)
{
	ShotMatrix = mat;
}

void CTank::Free()
{
	__super::Free();
	Safe_Release(m_VIBuffer);
	Safe_Release(m_CBBindingQuad);
	Safe_Release(m_VIBufferQuad);
	Safe_Release(m_QuadWorldTransform);
	Safe_Release(m_QuadTexTransform);
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


void CTank::RotPotap_And_Posin(float fTimeDelta)
{

	if (m_fPotapRotation <= m_fCamPotapRot - 0.5f)
		m_fPotapRotation += 20.f * fTimeDelta;
	else if (m_fPotapRotation >= m_fCamPotapRot + 0.5f)
		m_fPotapRotation -= 20.f * fTimeDelta;
	else
		m_fPotapRotation = m_fCamPotapRot;

	if (m_fPosinRotation <= m_fCamPosinRot - 0.5f)
		m_fPosinRotation += 20.f * fTimeDelta;
	else if (m_fPosinRotation >= m_fCamPosinRot + 0.5f)
		m_fPosinRotation -= 20.f * fTimeDelta;
	else
		m_fPosinRotation = m_fCamPosinRot;
}

void CTank::SendMyStateToServer()
{
	_float4x4 TempMat;
	XMStoreFloat4x4(&TempMat, m_TransformCom->Get_WorldMatrix());
	auto sendBuffer = ClientPacketHandler::Make_C_MOVE(TempMat, m_fPotapRotation, m_fPosinRotation);
	ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);
}

void CTank::SendShootDataToServer()
{
	_float4x4 TempMat;
	XMStoreFloat4x4(&TempMat, ShotMatrix);
	_vector vPos = ShotMatrix.r[3];
	_vector vDir = XMVector3Normalize(ShotMatrix.r[2]);

	_float3 fPos, fDir;
	XMStoreFloat3(&fPos, vPos);
	XMStoreFloat3(&fDir, vDir);


	float data[6] = {
	   fPos.x, fPos.y, fPos.z,  // 위치
	   fDir.x, fDir.y, fDir.z   // 정규화된 방향
	};


	auto sendBuffer = ClientPacketHandler::Make_C_SHOT(fPos.x, fPos.y, fPos.z,
		fDir.x, fDir.y, fDir.z);
	ServiceManager::GetInstace().GetService()->Broadcast(sendBuffer);

}

void CTank::PushBulletMatrix(const _matrix& mat)
{
	std::lock_guard<std::mutex> lock(BulletQueueMutex);
	BulletQueue.push(mat);
}

void CTank::PopAllBulletMatrix(std::function<void(const _matrix&)> processFunc)
{
	std::lock_guard<std::mutex> lock(BulletQueueMutex);
	while (!BulletQueue.empty())
	{
		const _matrix& mat = BulletQueue.front();
		processFunc(mat);
		BulletQueue.pop();
	}
}

void CTank::Set_OtherPlayerState(_float4x4 mat, float PotapRot, float PosinRot)
{

	m_TransformCom->Set_WorldMatrix(mat);
	Set_PotapRotation(PotapRot);
	Set_PoSinpRotation(PosinRot);

}

void CTank::Tick_For_Posin_Image(float fTimeDelta)
{
	m_RendererCom->AddtoRenderObjects(m_RGQuad, this);

	m_QuadWorldTransform->Identity();

	m_QuadWorldTransform->Set_Scale(CTransform::STATE_UP, 1.7777f);

	// 여기까지는 기본으로 해줘야 하는 것.

	float angleDiffX = m_fPotapRotation - m_fCamPotapRot;
	float angleDiffY = m_fCamPosinRot- m_fPosinRotation;

	// 스케일링.
	if (angleDiffX == 0.f && angleDiffY == 0.f) {
		m_fSameTime += fTimeDelta;
		if (m_fSameTime > 3.f)
			m_fSameTime = 3.f;
	}
	else
	{
		/*m_fSameTime -= fTimeDelta;
		if (m_fSameTime < 0)
			m_fSameTime = 0.f;*/
		m_fSameTime = 0.f;
	}

	float fScale = 1.f - m_fSameTime * 0.29f;

	m_QuadWorldTransform->Set_Scale(fScale);

	// 위치 바꾸기
	angleDiffX /= 30.f;
	angleDiffY /= 30.f;

	m_QuadWorldTransform->Set_State(CTransform::STATE_POSITION, XMVectorSet(angleDiffX, angleDiffY, 0, 1));

	m_CBBindingQuad->Set_World_TexCoord_And_Update(m_QuadWorldTransform, m_QuadTexTransform);
}

void CTank::Render_For_Posin_Image()
{
	m_CBBindingQuad->Set_On_Shader();

	m_VIBufferQuad->Render();
}
