#include "MyPhysicsEngine.h"

using namespace MyPhysicsEngine;

CMyPhysicsEngine* CMyPhysicsEngine::m_pInstance = nullptr;

PxVehiclePadSmoothingData gPadSmoothingData =
{
	{
		6.0f,	//rise rate eANALOG_INPUT_ACCEL=0,
		6.0f,	//rise rate eANALOG_INPUT_BRAKE,		
		6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE,	
		2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT,	
		2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT,	
	},
	{
		10.0f,	//fall rate eANALOG_INPUT_ACCEL=0
		10.0f,	//fall rate eANALOG_INPUT_BRAKE_LEFT	
		10.0f,	//fall rate eANALOG_INPUT_BRAKE_RIGHT	
		5.0f,	//fall rate eANALOG_INPUT_THRUST_LEFT	
		5.0f	//fall rate eANALOG_INPUT_THRUST_RIGHT
	}
};

PxVehicleDriveTankRawInputData gVehicleInputData(PxVehicleDriveTankControlModel::eSTANDARD);

MyPhysicsEngine::CMyPhysicsEngine::CMyPhysicsEngine()
{
}

void MyPhysicsEngine::CMyPhysicsEngine::Initialize_PhysX()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	PxU32 numWorkers = 1;
	gDispatcher = PxDefaultCpuDispatcherCreate(numWorkers);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = VehicleFilterShader;

	gScene = gPhysics->createScene(sceneDesc);
	
	gMaterial = gPhysics->createMaterial(0.8f, 0.8f, 0.6f);

	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

	PxInitVehicleSDK(*gPhysics);
	PxVehicleSetBasisVectors(PxVec3(0, 1, 0), PxVec3(0, 0, 1));
	PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);

	gVehicleSceneQueryData = VehicleSceneQueryData::allocate(1, PX_MAX_NB_WHEELS, 1, 1, WheelSceneQueryPreFilterBlocking, NULL, gAllocator);
	gBatchQuery = VehicleSceneQueryData::setUpBatchedSceneQuery(0, *gVehicleSceneQueryData, gScene);

	gFrictionPairs = createFrictionPairs(gMaterial);
}

void MyPhysicsEngine::CMyPhysicsEngine::Add_Terrain_From_File(const char* fileName, float scale, float heightScale)
{
	PxFilterData groundPlaneSimFilterData(COLLISION_FLAG_GROUND, COLLISION_FLAG_GROUND_AGAINST, 0, 0);
	//gGroundPlane = createDrivablePlane(groundPlaneSimFilterData, gMaterial, gPhysics);
	gGroundPlane = createDrivableTerrainFromImage(fileName, groundPlaneSimFilterData, gPhysics, gCooking, gMaterial);
	gScene->addActor(*gGroundPlane);
}

VehicleDesc MyPhysicsEngine::CMyPhysicsEngine::initTankDesc()
{
	const PxF32 chassisMass = 6200.0f;
	const PxVec3 chassisDims(3.8f, 1.f, 9.6f);
	const PxVec3 chassisMOI
	((chassisDims.y * chassisDims.y + chassisDims.z * chassisDims.z) * chassisMass / 12.0f,
		(chassisDims.x * chassisDims.x + chassisDims.z * chassisDims.z) * 0.8f * chassisMass / 12.0f,
		(chassisDims.x * chassisDims.x + chassisDims.y * chassisDims.y) * chassisMass / 12.0f);
	const PxVec3 chassisCMOffset(0.0f, -chassisDims.y * 0.5f + 0.65f, -0.1f);

	//Set up the wheel mass, radius, width, moment of inertia, and number of wheels.
	//Moment of inertia is just the moment of inertia of a cylinder.
	const PxF32 wheelMass = 60.0f;
	/*const PxF32 wheelRadius = 0.5f;
	const PxF32 wheelWidth = 0.6f;
	const PxF32 wheelMOI = 0.5f * wheelMass * wheelRadius * wheelRadius;
	const PxU32 nbWheels = 14;*/
	const PxF32 wheelRadius = 0.25f;  // 약 92cm 정도로 보임
	const PxF32 wheelWidth = 0.4f;    // 약 40cm 정도로 가정
	const PxF32 wheelMOI = 0.5f * wheelMass * wheelRadius * wheelRadius;
	const PxU32 nbWheels = 14;

	VehicleDesc tankDesc;

	tankDesc.chassisMass = chassisMass;
	tankDesc.chassisDims = chassisDims;
	tankDesc.chassisMOI = chassisMOI;
	tankDesc.chassisCMOffset = chassisCMOffset;
	tankDesc.chassisMaterial = gMaterial;
	tankDesc.chassisSimFilterData = PxFilterData(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);

	tankDesc.wheelMass = wheelMass;
	tankDesc.wheelRadius = wheelRadius;
	tankDesc.wheelWidth = wheelWidth;
	tankDesc.wheelMOI = wheelMOI;
	tankDesc.numWheels = nbWheels;
	tankDesc.wheelMaterial = gMaterial;
	tankDesc.chassisSimFilterData = PxFilterData(COLLISION_FLAG_WHEEL, COLLISION_FLAG_WHEEL_AGAINST, 0, 0);

	return tankDesc;
}

void MyPhysicsEngine::CMyPhysicsEngine::Add_Tank(float x, float y, float z)
{
	VehicleDesc tankDesc = initTankDesc();
	gTank = createVehicleTank(tankDesc, gPhysics, gCooking);
	//PxTransform startTransform(PxVec3(0, (tankDesc.chassisDims.y*0.5f + tankDesc.wheelRadius + 1.0f), 0), PxQuat(PxIdentity));
	PxTransform startTransform(PxVec3(x, y, z), PxQuat(PxIdentity));
	gTank->getRigidDynamicActor()->setGlobalPose(startTransform);
	gScene->addActor(*gTank->getRigidDynamicActor());

	gTank->setToRestState();
	gTank->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
	gTank->mDriveDynData.setUseAutoGears(true);
	gTank->setDriveModel(PxVehicleDriveTankControlModel::eSTANDARD);

	m_VehicleQueryResult.wheelQueryResults = m_WheelQueryResults;
}

void MyPhysicsEngine::CMyPhysicsEngine::Set_Pos(float x, float y, float z)
{
	PxTransform startTransform(PxVec3(x, y, z), PxQuat(PxIdentity));
	gTank->getRigidDynamicActor()->setGlobalPose(startTransform);
	gTank->setToRestState();
}

void MyPhysicsEngine::CMyPhysicsEngine::Set_Tank_ControlState(TankControlState TCS)
{
	gVehicleInputData.setAnalogAccel(
		(TCS.leftThrust || TCS.rightThrust || TCS.leftReverse || TCS.rightReverse) ? 1.0f : 0.0f);

	if (TCS.leftThrust || TCS.rightThrust)
		gTank->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
	else
		gTank->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);

	gVehicleInputData.setAnalogLeftThrust(TCS.leftThrust ? 1.0f : 0.0f);
	gVehicleInputData.setAnalogRightThrust(TCS.rightThrust ? 1.0f : 0.0f);
	gVehicleInputData.setAnalogLeftBrake(TCS.leftBrake ? 1.0f : 0.0f);
	gVehicleInputData.setAnalogRightBrake(TCS.rightBrake ? 1.0f : 0.0f);
}

void MyPhysicsEngine::CMyPhysicsEngine::Update_PhysX(float deltaTime)
{
	PxVehicleDriveTankSmoothAnalogRawInputsAndSetAnalogInputs(gPadSmoothingData, gVehicleInputData, deltaTime, *gTank);

	//Raycasts.
	PxVehicleWheels* vehicles[1] = { gTank };
	const PxU32 raycastQueryResultsSize = gVehicleSceneQueryData->getQueryResultBufferSize();
	PxRaycastQueryResult* raycastQueryResults = gVehicleSceneQueryData->getRaycastQueryResultBuffer(0);
	PxVehicleSuspensionRaycasts(gBatchQuery, 1, vehicles, raycastQueryResultsSize, raycastQueryResults);

	//Vehicle update.
	//m_VehicleQueryResult.wheelQueryResults = m_WheelQueryResults;
	const PxVec3 grav = gScene->getGravity();
	PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];
	PxVehicleWheelQueryResult vehicleQueryResults[1] = { {m_WheelQueryResults, gTank->mWheelsSimData.getNbWheels()} };
	m_VehicleQueryResult = vehicleQueryResults[0];
	PxVehicleUpdates(deltaTime, grav, *gFrictionPairs, 1, vehicles, vehicleQueryResults);

	//Scene update.
	gScene->simulate(deltaTime);
	gScene->fetchResults(true);
}

PxMat44 MyPhysicsEngine::CMyPhysicsEngine::Get_Tank_Transform(TankComponent TC)
{
	PxMat44 transform = PxMat44(PxIdentity);

	if (gTank != nullptr)
	{
		PxRigidDynamic* actor = gTank->getRigidDynamicActor();
		if (actor != nullptr)
		{
			// 차체 요청
			if (TC == TC_CHASSIS)
			{
				PxTransform pose = actor->getGlobalPose();
				transform = PxMat44(pose);
			}
			// 휠 요청
			else if (TC > TC_CHASSIS && TC < TC_COUNT)
			{
				PxU32 wheelIndex = static_cast<PxU32>(TC) - 1;

				if (wheelIndex < gTank->mWheelsSimData.getNbWheels())
				{
					const PxVehicleWheelQueryResult& wheelQuery = m_VehicleQueryResult;
					const PxWheelQueryResult& wheelResult = wheelQuery.wheelQueryResults[wheelIndex];

					// 휠 위치와 회전을 포함한 Transform
					const PxTransform wheelPose(wheelResult.localPose.p);

					// 차량 차체의 글로벌 Pose 기준으로 변환
					const PxTransform chassisPose = gTank->getRigidDynamicActor()->getGlobalPose();
					PxTransform globalWheelPose = chassisPose*wheelPose;

					//globalWheelPose.p *= 0.5f;
					//globalWheelPose.p.y += 1.5f;

					//globalWheelPose.p.y += 0.3f;

					transform = PxMat44(globalWheelPose);
					//transform = PxMat44(wheelPose);
				}
			}

		}
		return transform;
	}
}

void MyPhysicsEngine::CMyPhysicsEngine::Release_PhysX()
{
	gTank->getRigidDynamicActor()->release();
	gTank->free();
	PX_RELEASE(gGroundPlane);
	PX_RELEASE(gBatchQuery);
	gVehicleSceneQueryData->free(gAllocator);
	PX_RELEASE(gFrictionPairs);
	PxCloseVehicleSDK();

	PX_RELEASE(gMaterial);
	PX_RELEASE(gCooking);
	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);

	PX_RELEASE(gPhysics);
	
	PX_RELEASE(gFoundation);
}

CMyPhysicsEngine* MyPhysicsEngine::CMyPhysicsEngine::Get_Instance()
{
	if(m_pInstance == nullptr)
	{
		m_pInstance = new CMyPhysicsEngine();
	}
	return m_pInstance;
}

void MyPhysicsEngine::CMyPhysicsEngine::Release_Instance()
{
	if(m_pInstance != nullptr)
	{
		m_pInstance->Release_PhysX();
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}