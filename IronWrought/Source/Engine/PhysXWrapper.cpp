#include "PhysXWrapper.h"
#include "ContactReportCallback.h"
#include "Timer.h"
#include "Scene.h"
#include "Engine.h"
#include "RigidDynamicBody.h"
#include "CharacterController.h"

PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(filterData0);
	PX_UNUSED(filterData1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);

	// all initial and persisting reports for everything, with per-point data
	pairFlags = PxPairFlag::eCONTACT_DEFAULT
		| PxPairFlag::eNOTIFY_TOUCH_FOUND
		| PxPairFlag::eNOTIFY_TOUCH_PERSISTS
		| PxPairFlag::eNOTIFY_CONTACT_POINTS
		| PxPairFlag::eDETECT_CCD_CONTACT;

	return PxFilterFlag::eDEFAULT;
}

CPhysXWrapper::CPhysXWrapper()
{
	myFoundation = nullptr;
	myPhysics = nullptr;
	myDispatcher = nullptr;
	myPXMaterial = nullptr;
	myPhysicsVisualDebugger = nullptr;
	myAllocator = nullptr;
	myContactReportCallback = nullptr;
	//myControllerManager = nullptr;
}

CPhysXWrapper::~CPhysXWrapper()
{
	//I will fix later -- crashes because cant release nullptr //Alexander Matth�i 15/1 - 2021

	//myPXMaterial->release();
	//myDispatcher->release();
	//myPhysics->release();
	//myPhysicsVisualDebugger->release();
	//myFoundation->release();
	//delete myAllocator;
	//myAllocator = nullptr;
	myPhysicsVisualDebugger->disconnect();
}

bool CPhysXWrapper::Init()
{
	PxDefaultErrorCallback errorCallback;

	myAllocator = new PxDefaultAllocator();
	myFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myAllocator, errorCallback);
	if (!myFoundation) {
		return false;
	}
	myPhysicsVisualDebugger = PxCreatePvd(*myFoundation);
	if (!myPhysicsVisualDebugger) {
		return false;
	}
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
	//PxPvdTransport* transport = PxDefaultPvdFileTransportCreate("Test.pxd2");
	myPhysicsVisualDebugger->connect(*transport, PxPvdInstrumentationFlag::eALL);

	myPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *myFoundation, PxTolerancesScale(), true, myPhysicsVisualDebugger);
	if (!myPhysics) {
		return false;
	}

	// All collisions gets pushed to this class
	myContactReportCallback = new CContactReportCallback();
    return true;
}

physx::PxScene* CPhysXWrapper::CreatePXScene(CScene* aScene)
{
	PxSceneDesc sceneDesc(myPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.82f, 0.0f);
	myDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = myDispatcher;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = myContactReportCallback;
	PxScene* pXScene = myPhysics->createScene(sceneDesc);
	if (!pXScene) {
		return false;
	}

	PxPvdSceneClient* pvdClient = pXScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	} 

	// Create a basic setup for a scene - contain the rodents in a invisible cage
	PxMaterial* myMaterial = myPhysics->createMaterial(1.0f, 0.0f, -0.5f);

	PxRigidStatic* groundPlane = PxCreatePlane(*myPhysics, PxPlane(0, 1, 0, 3.3f), *myMaterial);
	//groundPlane->setGlobalPose( {15.0f,0.0f,0.0f} );
	pXScene->addActor(*groundPlane);

	myControllerManagers[pXScene] = PxCreateControllerManager(*pXScene);

	myPXScenes[aScene] = pXScene;

	return pXScene;
}

PxScene* CPhysXWrapper::GetPXScene()
{
	return myPXScenes[&CEngine::GetInstance()->GetActiveScene()];
}

void CPhysXWrapper::Simulate()
{
	if (GetPXScene() != nullptr) {
		GetPXScene()->simulate(CTimer::Dt());
		GetPXScene()->fetchResults(true);
	}
}

CRigidDynamicBody* CPhysXWrapper::CreateDynamicRigidbody(const Vector3& aPos)
{
	CRigidDynamicBody* dynamicBody = new CRigidDynamicBody(*myPhysics, aPos);
	GetPXScene()->addActor(dynamicBody->GetBody());
	return dynamicBody;
}

CCharacterController* CPhysXWrapper::CreateCharacterController(PxControllerShapeType::Enum aType, const Vector3& aPos, const float& aRadius, const float& aHeight)
{
	CCharacterController* characterController = new CCharacterController(aType, aPos, aRadius, aHeight);
	return characterController;
}

PxControllerManager* CPhysXWrapper::GetControllerManger()
{
	return myControllerManagers[GetPXScene()];
}
