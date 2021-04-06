#include "stdafx.h"
#include "BoxColliderComponent.h"
#include "PhysXWrapper.h"
#include "RigidBodyComponent.h"
#include "Engine.h"
#include "Scene.h"
#include "RigidDynamicBody.h"
#include "TransformComponent.h"

CBoxColliderComponent::CBoxColliderComponent(CGameObject& aParent, const Vector3& aPositionOffset, const Vector3& aBoxSize, bool aIsStatic)
	: CBehaviour(aParent)
	, myShape(nullptr)
	, myPositionOffset(aPositionOffset)
	, myBoxSize(aBoxSize)
	, created(false)
{
	aIsStatic;
}

CBoxColliderComponent::~CBoxColliderComponent()
{
	created = false;
}

void CBoxColliderComponent::Awake()
{
	myShape = CEngine::GetInstance()->GetPhysx().GetPhysics()->createShape(physx::PxBoxGeometry(myBoxSize.x / 2.f, myBoxSize.y / 2.f, myBoxSize.z / 2.f), *CEngine::GetInstance()->GetPhysx().CreateMaterial(CPhysXWrapper::materialfriction::metal), true);
	myShape->setLocalPose({ myPositionOffset.x, myPositionOffset.y, myPositionOffset.z });
	myShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	CRigidBodyComponent* rigidBody = nullptr;
	if (GameObject().TryGetComponent(&rigidBody))
	{
		rigidBody->AttachShape(myShape);
		physx::PxRigidDynamic& dynamic = rigidBody->GetDynamicRigidBody()->GetBody();
		dynamic.setMass(3.f);
		dynamic.setCMassLocalPose({ myPositionOffset.x, myPositionOffset.y, myPositionOffset.z });
		dynamic.putToSleep();
		dynamic.setMaxLinearVelocity(10.f);
		created = true;
	}
	else
	{
		DirectX::SimpleMath::Vector3 translation;
		DirectX::SimpleMath::Vector3 scale;
		DirectX::SimpleMath::Quaternion quat;
		DirectX::SimpleMath::Matrix transform = GameObject().GetComponent<CTransformComponent>()->GetLocalMatrix();
		transform.Decompose(scale, quat, translation);

		PxVec3 pos = { translation.x, translation.y, translation.z };
		PxQuat pxQuat = { quat.x, quat.y, quat.z, quat.w };
		CEngine::GetInstance()->GetPhysx().GetPhysics()->createRigidStatic({ pos, pxQuat });
	}
}

void CBoxColliderComponent::Start()
{
}

void CBoxColliderComponent::Update()
{
}

void CBoxColliderComponent::OnEnable()
{
}

void CBoxColliderComponent::OnDisable()
{
}
