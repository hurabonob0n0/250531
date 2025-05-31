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
	const PxVec3 chassisDims(3.8f, 1.5f, 9.6f);
	const PxVec3 chassisMOI
	((chassisDims.y * chassisDims.y + chassisDims.z * chassisDims.z) * chassisMass / 12.0f,
		(chassisDims.x * chassisDims.x + chassisDims.z * chassisDims.z) * 0.8f * chassisMass / 12.0f,
		(chassisDims.x * chassisDims.x + chassisDims.y * chassisDims.y) * chassisMass / 12.0f);
	const PxVec3 chassisCMOffset(0.0f, -chassisDims.y * 0.5f + 0.65f, -0.1f);

	//Set up the wheel mass, radius, width, moment of inertia, and number of wheels.
	//Moment of inertia is just the moment of inertia of a cylinder.
	const PxF32 wheelMass = 60.0f;
	const PxF32 wheelRadius = 0.5f;
	const PxF32 wheelWidth = 0.6f;
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
}

void MyPhysicsEngine::CMyPhysicsEngine::Set_Tank_ControlState(TankControlState TCS)
{
	gVehicleInputData.setAnalogAccel(
		(TCS.leftThrust || TCS.rightThrust) ? 1.0f : 0.0f);

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
	const PxVec3 grav = gScene->getGravity();
	PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];
	PxVehicleWheelQueryResult vehicleQueryResults[1] = { {wheelQueryResults, gTank->mWheelsSimData.getNbWheels()} };
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
				// 휠 인덱스 확인
				const PxU32 wheelIndex = static_cast<PxU32>(TC) - 1;

				// 휠이 존재하는지 확인
				if (wheelIndex < gTank->mWheelsSimData.getNbWheels())
				{
					// 휠 중심 위치 계산
					const PxVec3 suspTravelDir = gTank->mWheelsSimData.getSuspTravelDirection(wheelIndex);
					const PxVec3 suspOffset = gTank->mWheelsSimData.getSuspForceAppPointOffset(wheelIndex);
					const PxVec3 wheelOffset = gTank->mWheelsSimData.getWheelCentreOffset(wheelIndex);

					const PxTransform chassisPose = actor->getGlobalPose();
					const PxVec3 worldSuspPos = chassisPose.transform(wheelOffset);

					// 휠 회전은 PxWheelQueryResult를 이용해야 정확하지만,
					// 여기서는 위치만 변환 행렬로 줄게.
					transform = PxMat44(PxTransform(worldSuspPos));
				}
			}
		}
	}

	return transform;
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