#pragma once
#include <PxPhysicsAPI.h>

using namespace physx;

class CContactReportCallback;
class CRigidDynamicBody;
class CScene;
class CCharacterController;

class CPhysXWrapper
{
public:

	enum class materialfriction
	{
		metal,
		wood,
		bounce,
		basic,
		none


	};

public:

	CPhysXWrapper();
	~CPhysXWrapper();

	bool Init();

	bool CreatePXScene(CScene* aScene);
	PxScene* GetPXScene();
	PxPhysics* GetPhysics() { return myPhysics; }


	PxRaycastBuffer Raycast(Vector3 origin, Vector3 direction, float distance);
	void RaycastHit(PxVec3 position, PxVec3 normal);


	PxMaterial* CreateMaterial(materialfriction amaterial);

	void Simulate();

	CRigidDynamicBody* CreateDynamicRigidbody(const Vector3& aPos);

	CCharacterController* CreateCharacterController(PxControllerShapeType::Enum aType, const Vector3& aPos, const float& aRadius, const float& aHeight);

	PxControllerManager* GetControllerManger();


  //merge conflict 8/3/2021
	//void DebugLines();
	//void Cooking(std::vector<CGameObject*> gameObjectsToCook, CScene* aScene);

private:


	PxFoundation* myFoundation;
	PxPhysics* myPhysics;
	PxDefaultCpuDispatcher* myDispatcher;
	PxMaterial* myPXMaterial;
	PxPvd* myPhysicsVisualDebugger;
	PxDefaultAllocator* myAllocator;
		//merge conflicttttt 8/3/2021
		//PxCooking* myCooking;
	CContactReportCallback* myContactReportCallback;
	//PxControllerManager* myControllerManager;
	std::unordered_map<PxScene*, PxControllerManager*> myControllerManagers;
	std::unordered_map<CScene*, PxScene*> myPXScenes;
};
