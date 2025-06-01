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

	//CModel* pModel = (CModel*)m_GameInstance->Get_Component("ModelCom");
	m_VIBuffer = (CModel*)m_GameInstance->Get_Component("ModelCom");

	MaterialData mat{};
	mat.DiffuseMapIndex = m_GameInstance->Add_Texture("Tank_Glacis_D", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glacis_Plate_Base_color.dds"));
	mat.NormalMapIndex = m_GameInstance->Add_Texture("Tank_Glacis_N", CTexture::Create(L"../bin/Models/TankDDS/M1A2_Glacis_Plate_Normal.dds"));
	m_VIBuffer->Set_MatOffsets(mat.DiffuseMapIndex); // Camera를 먼저 추가한다고 가정.
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
	if (_myPlayer)
	{
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
	}
	//if (m_GameInstance->Key_Pressing(VK_RIGHT))
	//    m_TestX += 10.f * fTimeDelta;

	//if (m_GameInstance->Key_Pressing(VK_LEFT))
	//    m_TestX -= 10.f * fTimeDelta;

	//if (m_GameInstance->Key_Pressing(VK_UP))
	//    m_TestZ += 10.f * fTimeDelta;

	//if (m_GameInstance->Key_Pressing(VK_DOWN))
	//    m_TestZ -= 10.f * fTimeDelta;
	//    //m_fPosinRotation += 1.f * fTimeDelta;

	//_float4x4 matworld;
	//XMStoreFloat4x4(&matworld, XMMatrixIdentity());

	//m_TransformCom->Set_WorldMatrix(matworld);

	//m_TransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_TestX, 100.f, m_TestZ, 1.f));

	else
	{
		if (m_GameInstance->Key_Pressing('W'))
			m_TransformCom->Go_Straight(fTimeDelta);

		if (m_GameInstance->Key_Pressing('A'))
			m_TransformCom->Go_Left(fTimeDelta);

		if (m_GameInstance->Key_Pressing('S'))
			m_TransformCom->Go_Backward(fTimeDelta);

		if (m_GameInstance->Key_Pressing('D'))
			m_TransformCom->Go_Right(fTimeDelta);
	}



}

void CTank::LateTick(float fTimeDelta)
{
	__super::LateTick(fTimeDelta);

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
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_SECOND_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R1Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_THIRD_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L2Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_FOURTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R2Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_FIFTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L3Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_SIXTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R3Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_LEFT_SEVENTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L4Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_FIRST_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R4Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_SECOND_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L5Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_THIRD_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R5Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_FOURTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L6Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_FIFTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R6Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_SIXTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX L7Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);

		pxMat = m_pPhysicsEngine->Get_Tank_Transform(MyPhysicsEngine::CMyPhysicsEngine::TC_RIGHT_SEVENTH_WHEEL);

		// PxMat44은 row-major 이므로 열 단위로 XMVECTOR를 생성
		XMMATRIX R7Mat = XMMATRIX(
			XMVectorSet(pxMat.column0.x, pxMat.column0.y, pxMat.column0.z, pxMat.column0.w),
			XMVectorSet(pxMat.column1.x, pxMat.column1.y, pxMat.column1.z, pxMat.column1.w),
			XMVectorSet(pxMat.column2.x, pxMat.column2.y, pxMat.column2.z, pxMat.column2.w),
			XMVectorSet(pxMat.column3.x, pxMat.column3.y, pxMat.column3.z, pxMat.column3.w)
		);
#pragma endregion



		XMMATRIX matPotap = XMMatrixRotationY(m_fPotapRotation);

		XMMATRIX matPosin = XMMatrixRotationY(m_fPosinRotation);


		m_VIBuffer->Set_Transform_Matrix(0, mat); // Chassis
		m_VIBuffer->Set_Transform_Matrix(1, matPotap); // Potap


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
	}

	else
	{
		m_VIBuffer->Set_Transform_Matrix(0, m_TransformCom->Get_WorldMatrix());
		m_VIBuffer->Invalidate_Bones();
		m_VIBuffer->Update();
	}

}

void CTank::Render()
{
	for (int i = 0; i < 55; ++i)
		m_VIBuffer->Render(i);
}

void CTank::Set_PotapRotation(float fDegree)
{
	m_fPotapRotation = fDegree;
}

void CTank::Set_PoSinpRotation(float fDegree)
{
	m_fPosinRotation = fDegree;
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
