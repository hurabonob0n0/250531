#pragma once

#include "PxPhysicsAPI.h"

#include "../snippetvehiclecommon/SnippetVehicleSceneQuery.h"
#include "../snippetvehiclecommon/SnippetVehicleFilterShader.h"
#include "../snippetvehiclecommon/SnippetVehicleTireFriction.h"
#include "../snippetvehiclecommon/SnippetVehicleCreate.h"

#include "../snippetutils/SnippetUtils.h"

using namespace physx;
using namespace snippetvehicle;

namespace MyPhysicsEngine
{


class CMyPhysicsEngine
{
public:
	enum TankComponent
	{
		TC_CHASSIS = 0,
		TC_LEFT_FIRST_WHEEL = 1,
		TC_LEFT_SECOND_WHEEL = 2,
		TC_LEFT_THIRD_WHEEL = 3,
		TC_LEFT_FOURTH_WHEEL = 4,
		TC_LEFT_FIFTH_WHEEL = 5,
		TC_LEFT_SIXTH_WHEEL = 6,
		TC_LEFT_SEVENTH_WHEEL = 7,
		TC_RIGHT_FIRST_WHEEL = 8,
		TC_RIGHT_SECOND_WHEEL = 9,
		TC_RIGHT_THIRD_WHEEL = 10,
		TC_RIGHT_FOURTH_WHEEL = 11,
		TC_RIGHT_FIFTH_WHEEL = 12,
		TC_RIGHT_SIXTH_WHEEL = 13,
		TC_RIGHT_SEVENTH_WHEEL = 14,
		TC_COUNT = 15
	};

struct TankControlState {
	bool leftThrust = false;
	bool rightThrust = false;
	bool leftBrake = false;
	bool rightBrake = false;
	bool leftReverse = false;
	bool rightReverse = false;
};

private:
	CMyPhysicsEngine();
	CMyPhysicsEngine(const CMyPhysicsEngine&) = delete;
	CMyPhysicsEngine& operator=(const CMyPhysicsEngine&) = delete;
	~CMyPhysicsEngine() = default;

public:
	void Initialize_PhysX();
	void Add_Terrain_From_File(const char* fileName, float scale = 1.0f, float heightScale = 1.0f);
	VehicleDesc initTankDesc();
	void Add_Tank(float x, float y, float z);
	void Set_Tank_ControlState(TankControlState TCS);
	void Update_PhysX(float deltaTime);
	PxMat44 Get_Tank_Transform(TankComponent TC);
	void Release_PhysX();

private:
	PxDefaultAllocator		gAllocator;
	PxDefaultErrorCallback	gErrorCallback;

	PxFoundation* gFoundation = NULL;
	PxPhysics* gPhysics = NULL;

	PxDefaultCpuDispatcher* gDispatcher = NULL;
	PxScene* gScene = NULL;

	PxCooking* gCooking = NULL;

	PxMaterial* gMaterial = NULL;

	VehicleSceneQueryData* gVehicleSceneQueryData = NULL;
	PxBatchQuery* gBatchQuery = NULL;

	PxVehicleDrivableSurfaceToTireFrictionPairs* gFrictionPairs = NULL;

	PxRigidStatic* gGroundPlane = NULL;
	PxVehicleDriveTank* gTank = NULL;

	TankControlState m_TankControlState;

public:
	static CMyPhysicsEngine* Get_Instance();
	static void Release_Instance();

private:
	static CMyPhysicsEngine* m_pInstance;
};
}