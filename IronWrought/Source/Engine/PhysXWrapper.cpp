#include "PhysXWrapper.h"
#include "ContactReportCallback.h"
#include "Timer.h"
#include "Scene.h"
#include "Engine.h"
#include "RigidDynamicBody.h"
#include <iostream>
#include "LineFactory.h"
#include "LineInstance.h"
#include "ModelComponent.h"
#include "InstancedModelComponent.h"
#include "ModelFactory.h"
#include "Model.h"
#include "CharacterController.h"
#include <PlayerControllerComponent.h>

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
	PxTolerancesScale scale;
	scale.length = 1;
	myCooking = PxCreateCooking(PX_PHYSICS_VERSION, *myFoundation, PxCookingParams(scale));

	myPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *myFoundation, PxTolerancesScale(), true, myPhysicsVisualDebugger);
	if (!myPhysics) {
		return false;
	}

	// All collisions gets pushed to this class
	myContactReportCallback = new CContactReportCallback();
    return true;
}

PxScene* CPhysXWrapper::CreatePXScene(CScene* aScene)
{
	PxSceneDesc sceneDesc(myPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.82f, 0.0f);
	myDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = myDispatcher;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = myContactReportCallback;
	PxScene* pXScene = myPhysics->createScene(sceneDesc);
	if (!pXScene) {
		return nullptr;
	}

	PxPvdSceneClient* pvdClient = pXScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	// Create a basic setup for a scene - contain the rodents in a invisible cage
	/*PxMaterial* myMaterial*/myPXMaterial = CreateMaterial(CPhysXWrapper::materialfriction::basic);

	PxRigidStatic* groundPlane = PxCreatePlane(*myPhysics, PxPlane(0, 1, 0, 3.3f), *myPXMaterial/**myMaterial*/);
	//groundPlane->setGlobalPose( {15.0f,0.0f,0.0f} );
	pXScene->addActor(*groundPlane);

//pXScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
//pXScene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 2.0f);

	myControllerManagers[pXScene] = PxCreateControllerManager(*pXScene);

	myPXScenes[aScene] = pXScene;

	return pXScene;
}

PxScene* CPhysXWrapper::GetPXScene()
{
	return myPXScenes[&CEngine::GetInstance()->GetActiveScene()];
}

PxRaycastBuffer CPhysXWrapper::Raycast(Vector3 aOrigin, Vector3 aDirection, float /*aDistance*/)
{
	//PxScene* scene = CEngine::GetInstance()->GetActiveScene().PXScene();
	PxVec3 origin;
	origin.x = aOrigin.x;
	origin.y = aOrigin.y;
	origin.z = aOrigin.z;


	PxVec3 unitDir;

	unitDir.x = aDirection.x;
	unitDir.y = aDirection.y;
	unitDir.z = aDirection.z;

	//PxReal maxDistance = aDistance;
	PxRaycastBuffer hit;

	/*scene->raycast(origin, unitDir, maxDistance, hit);

		RaycastHit(hit.block.position, hit.block.normal);*/



	return hit;
}

void CPhysXWrapper::RaycastHit(PxVec3 position, PxVec3 normal)
{
	std::cout << "x: " << position.x << std::endl;
	std::cout << "y: " << position.y << std::endl;
	std::cout << "z: " << position.z << std::endl << std::endl;


	std::cout << normal.x << std::endl;

}





PxMaterial* CPhysXWrapper::CreateMaterial(materialfriction amaterial)
{
	switch ((materialfriction)amaterial)
	{
	case materialfriction::metal:
		return myPhysics->createMaterial(1.0f, 1.0f, 0.0f);
		break;
	case materialfriction::wood:
		return myPhysics->createMaterial(0.2f, 0.5f, 0.3f);

		break;
	case materialfriction::bounce:
		return myPhysics->createMaterial(0.0f, 0.0f, 1.0f);
		break;
	case materialfriction::basic:
		return myPhysics->createMaterial(0.5f, 0.5f, 0.5f);
		break;
	case materialfriction::none:
		return myPhysics->createMaterial(0.0f, 0.0f, 0.0f);
		break;
	default:

		break;
	}
	return nullptr;
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

//void CPhysXWrapper::DebugLines()
//{
//	const PxRenderBuffer& rb = CEngine::GetInstance()->GetActiveScene().PXScene()->getRenderBuffer();
//	for (PxU32 i = 0; i < rb.getNbLines(); i++)
//	{
//		const PxDebugLine& line = rb.getLines()[i];
//		CLineInstance* myLine = new CLineInstance();
//		myLine->Init(CLineFactory::GetInstance()->CreateLine({ line.pos0.x, line.pos0.y, line.pos0.z }, { line.pos1.x, line.pos1.y, line.pos1.z }, { 0.1f, 255.0f, 0.1f, 1.0f }));
//		CEngine::GetInstance()->GetActiveScene().AddInstance(myLine);
//	}
//}

void CPhysXWrapper::Cooking(std::vector<CGameObject*> gameObjectsToCook, CScene* aScene)
{
	for (int i = 0; i < gameObjectsToCook.size(); ++i) {
		if (gameObjectsToCook[i]->GetComponent<CModelComponent>() && !gameObjectsToCook[i]->GetComponent<CPlayerControllerComponent>()) {
			std::vector<PxVec3> verts(gameObjectsToCook[i]->GetComponent<CModelComponent>()->GetMyModel()->GetModelData().myMeshFilter.myVertecies.size());
			for (auto y = 0; y < gameObjectsToCook[i]->GetComponent<CModelComponent>()->GetMyModel()->GetModelData().myMeshFilter.myVertecies.size(); ++y) {
				Vector3 vec = gameObjectsToCook[i]->GetComponent<CModelComponent>()->GetMyModel()->GetModelData().myMeshFilter.myVertecies[y];
				verts[y] = PxVec3(vec.x, vec.y, vec.z);
			}

			PxTriangleMeshDesc meshDesc;
			meshDesc.points.count = (PxU32)verts.size();
			meshDesc.points.stride = sizeof(PxVec3);
			meshDesc.points.data = verts.data();

			std::vector<unsigned int> myIndexes = gameObjectsToCook[i]->GetComponent<CModelComponent>()->GetMyModel()->GetModelData().myMeshFilter.myIndexes;
			meshDesc.triangles.count = (PxU32)myIndexes.size() / (PxU32)3;
			meshDesc.triangles.stride = 3 * sizeof(PxU32);
			meshDesc.triangles.data = myIndexes.data();

			PxDefaultMemoryOutputStream writeBuffer;
			PxTriangleMeshCookingResult::Enum result;
			myCooking->cookTriangleMesh(meshDesc, writeBuffer, &result);

			PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
			PxTriangleMesh* pxMesh = myPhysics->createTriangleMesh(readBuffer);

			PxTriangleMeshGeometry pMeshGeometry(pxMesh);

			PxRigidStatic* actor = myPhysics->createRigidStatic({ 0.f, 0.f, 0.f });
			PxShape* shape = myPhysics->createShape(pMeshGeometry, *myPXMaterial, true);
			actor->attachShape(*shape);
			actor->setGlobalPose({ gameObjectsToCook[i]->myTransform->Position().x,gameObjectsToCook[i]->myTransform->Position().y, gameObjectsToCook[i]->myTransform->Position().z });
			aScene->PXScene()->addActor(*actor);

		}
		else if (gameObjectsToCook[i]->GetComponent<CInstancedModelComponent>() && !gameObjectsToCook[i]->GetComponent<CPlayerControllerComponent>()) {
			for (auto z = 0; z < gameObjectsToCook[i]->GetComponent<CInstancedModelComponent>()->GetInstancedTransforms().size(); ++z) {
				std::vector<PxVec3> verts(gameObjectsToCook[i]->GetComponent<CInstancedModelComponent>()->GetModel()->GetModelInstanceData().myMeshFilter.myVertecies.size());
				for (auto y = 0; y < gameObjectsToCook[i]->GetComponent<CInstancedModelComponent>()->GetModel()->GetModelInstanceData().myMeshFilter.myVertecies.size(); ++y) {
					Vector3 vec = gameObjectsToCook[i]->GetComponent<CInstancedModelComponent>()->GetModel()->GetModelInstanceData().myMeshFilter.myVertecies[y];
					verts[y] = PxVec3(vec.x, vec.y, vec.z);
				}

				PxTriangleMeshDesc meshDesc;
				meshDesc.points.count = (PxU32)verts.size();
				meshDesc.points.stride = sizeof(PxVec3);
				meshDesc.points.data = verts.data();

				std::vector<unsigned int> myInstancedIndexes = gameObjectsToCook[i]->GetComponent<CInstancedModelComponent>()->GetModel()->GetModelInstanceData().myMeshFilter.myIndexes;
				meshDesc.triangles.count = (PxU32)myInstancedIndexes.size() / (PxU32)3;
				meshDesc.triangles.stride = 3 * sizeof(PxU32);
				meshDesc.triangles.data = myInstancedIndexes.data();

				PxDefaultMemoryOutputStream writeBuffer;
				PxTriangleMeshCookingResult::Enum result;
				myCooking->cookTriangleMesh(meshDesc, writeBuffer, &result);

				PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
				PxTriangleMesh* pxMesh = myPhysics->createTriangleMesh(readBuffer);

				PxTriangleMeshGeometry pMeshGeometry(pxMesh);

				PxRigidStatic* actor = myPhysics->createRigidStatic({ 0.f, 0.f, 0.f });
				PxShape* shape = myPhysics->createShape(pMeshGeometry, *myPXMaterial, true);
				actor->attachShape(*shape);
				aScene->PXScene()->addActor(*actor);

				DirectX::SimpleMath::Vector3 translation;
				DirectX::SimpleMath::Vector3 scale;
				DirectX::SimpleMath::Quaternion quat;
				DirectX::SimpleMath::Matrix transform = gameObjectsToCook[i]->GetComponent<CInstancedModelComponent>()->GetInstancedTransforms()[z];
				transform.Decompose(scale, quat, translation);

				PxVec3 pos = { translation.x, translation.y, translation.z };
				PxQuat pxQuat = { quat.x, quat.y, quat.z, quat.w };
				actor->setGlobalPose({ pos, pxQuat });
			}
		}
	}
}