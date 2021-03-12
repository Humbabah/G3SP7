#include "stdafx.h"
#include "RigidBodyComponent.h"
#include "PhysXWrapper.h"
#include "RigidDynamicBody.h"
#include "Engine.h"


CRigidBodyComponent::CRigidBodyComponent(CGameObject& aParent, bool isStatic) 
	: CComponent(aParent)
{
	if (isStatic == false) {
		myDynamicRigidBody = CEngine::GetInstance()->GetPhysx().CreateDynamicRigidbody(aParent.GetComponent<CTransformComponent>()->Position(), aParent.InstanceID());
	}
	else {
		myDynamicRigidBody = nullptr;
	}
}

CRigidBodyComponent::~CRigidBodyComponent()
{
	myDynamicRigidBody = nullptr;
}

void CRigidBodyComponent::Awake()
{
}

void CRigidBodyComponent::Start()
{
}

void CRigidBodyComponent::Update()
{
	GameObject().myTransform->Position(myDynamicRigidBody->GetPosition());
}
