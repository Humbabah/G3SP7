#include "stdafx.h"
#include "PlayerControllerComponent.h"

#include "GameObject.h"
#include "TransformComponent.h"

#include "InputMapper.h"

#include "PhysXWrapper.h"
#include "CharacterController.h"

#include "Scene.h"
#include "CameraControllerComponent.h"

#include "PlayerAnimationController.h"
#include "PlayerComponent.h"

#include "RigidBodyComponent.h"
#include "RigidDynamicBody.h"

// TEMP
static const float gPretendObjectDistanceFromPlayer = 10.0f;// TEMP
// TEMP
static float gPretendObjectCurrentDistance = 0.0f;// TEMP

CPlayerControllerComponent::CPlayerControllerComponent(CGameObject& gameObject, const float aWalkSpeed, const float aCrouchSpeed, physx::PxUserControllerHitReport* aHitReport)
	: CComponent(gameObject)
	, mySpeed(aWalkSpeed)
	, myIsCrouching(false)
	, myWalkSpeed(aWalkSpeed)
	, myCrouchSpeed(aCrouchSpeed)
	, myIsGrounded(true)
	, myHasJumped(false)
	, myIsJumping(false)
	, myJumpHeight(0.4f)
	, myFallSpeed(0.098f)
	, myMovement( Vector3(0.0f, -0.098f, 0.0f ))
{
	INPUT_MAPPER->AddObserver(EInputEvent::MoveForward,		this);
	INPUT_MAPPER->AddObserver(EInputEvent::MoveBackward,	this);
	INPUT_MAPPER->AddObserver(EInputEvent::MoveLeft,		this);
	INPUT_MAPPER->AddObserver(EInputEvent::MoveRight,		this);
	INPUT_MAPPER->AddObserver(EInputEvent::Jump, this);
	INPUT_MAPPER->AddObserver(EInputEvent::Crouch, this);
	INPUT_MAPPER->AddObserver(EInputEvent::Pull, this);
	INPUT_MAPPER->AddObserver(EInputEvent::Push, this);
	INPUT_MAPPER->AddObserver(EInputEvent::ResetEntities, this);
	INPUT_MAPPER->AddObserver(EInputEvent::SetResetPointEntities, this);

	myController = CEngine::GetInstance()->GetPhysx().CreateCharacterController(gameObject.myTransform->Position(), myColliderRadius, myColliderHeightStanding, GameObject().myTransform, aHitReport);
	GameObject().myTransform->Position(myController->GetPosition());// This is a test / Aki 2021 03 12

	GameObject().myTransform->FetchChildren()[0]->Position({ 0.0f, myCameraPosYStanding, myCameraPosZ });
	GameObject().myTransform->FetchChildren()[0]->Rotation({ 0.0f, 0.0f, 0.0f });
	myCamera = GameObject().myTransform->FetchChildren()[0]->GameObject().GetComponent<CCameraControllerComponent>();

	myAnimationComponentController = new CPlayerAnimationController();
	CAnimationComponent* animComp = GameObject().myTransform->FetchChildren()[0]->FetchChildren()[0]->GameObject().GetComponent<CAnimationComponent>();
	myAnimationComponentController->Init(animComp);
}

CPlayerControllerComponent::~CPlayerControllerComponent()
{
	INPUT_MAPPER->RemoveObserver(EInputEvent::MoveForward,	this);
	INPUT_MAPPER->RemoveObserver(EInputEvent::MoveBackward, this);
	INPUT_MAPPER->RemoveObserver(EInputEvent::MoveLeft,		this);
	INPUT_MAPPER->RemoveObserver(EInputEvent::MoveRight,	this);
	INPUT_MAPPER->RemoveObserver(EInputEvent::Jump, this);
	INPUT_MAPPER->RemoveObserver(EInputEvent::Crouch, this);
	INPUT_MAPPER->RemoveObserver(EInputEvent::Pull, this);
	INPUT_MAPPER->RemoveObserver(EInputEvent::Push, this);
	INPUT_MAPPER->RemoveObserver(EInputEvent::ResetEntities, this);
	INPUT_MAPPER->RemoveObserver(EInputEvent::SetResetPointEntities, this);

	delete myAnimationComponentController;
	myAnimationComponentController = nullptr;
}

void CPlayerControllerComponent::Awake()
{}

void CPlayerControllerComponent::Start()
{
	myRespawnPosition = myController->GetPosition();

}

void CPlayerControllerComponent::Update()
{
#ifdef _DEBUG
	if (myCamera->IsFreeCamMode() || myCamera->IsCursorUnlocked())
		return;
#endif

	if (myLadderHasTriggered)
	{
		LadderUpdate();
	}
	else
	{
		//Move({0.0f, myMovement.y, 0.0f});
	}

	//Move(myMovement * mySpeed);

	/*if (myPlayerComponent->getIsAlive() == false)
	{
		myController->SetPosition(myRespawnPosition);
		GameObject().myTransform->Position(myController->GetPosition());

		myPlayerComponent->setIsAlive(true);
		myPlayerComponent->resetHealth();
	}*/

	if (myHasJumped == true)
	{
		myMovement.y = myJumpHeight;
		myHasJumped = false;
	}

	if (myMovement.y >= -0.1f)
	{
		myMovement.y -= myFallSpeed * CTimer::FixedDt();
	}

	if (myIsJumping == false)
	{
		myMovement.y = myMovement.y >-0.0f ? myMovement.y - myFallSpeed : myMovement.y;
	}
	//std::cout << "Velocity X: " <<  myController->GetController().getActor()->getLinearVelocity().x << "Velocity Y: " << myController->GetController().getActor()->getLinearVelocity().y << "Velocity Z: " << myController->GetController().getActor()->getLinearVelocity().z << std::endl;;
	GameObject().myTransform->Position(myController->GetPosition());
	gPretendObjectCurrentDistance = max(gPretendObjectCurrentDistance -  CTimer::Dt() * 12.0f, 0.0f);
	myAnimationComponentController->UpdateBlendValue(min(gPretendObjectCurrentDistance / gPretendObjectDistanceFromPlayer, 1.0f));
	myAnimationComponentController->Update();

#ifdef _DEBUG
	if (Input::GetInstance()->IsKeyPressed('R'))
	{
		myController->SetPosition(myRespawnPosition);
	}
#endif // _DEBUG
	ControllerUpdate();
	Move(myMovement * mySpeed);
	//myMovement = { 0,0,0 };
}

void CPlayerControllerComponent::ReceiveEvent(const EInputEvent aEvent)
{
	/*CCameraControllerComponent* cameraController = CEngine::GetInstance()->GetActiveScene().FindFirstObjectWithComponent<CCameraControllerComponent>();*/
#ifdef _DEBUG
	if (myCamera->IsFreeCamMode() || myCamera->IsCursorUnlocked())
	{
		if (aEvent == EInputEvent::SetResetPointEntities)
			myRespawnPosition = myCamera->GameObject().myTransform->Position();

		return;
	}
#endif

	float y = myMovement.y;

	switch (aEvent)
	{
		case EInputEvent::MoveForward:
			//myMovement = -myCamera->GameObject().myTransform->GetLocalMatrix().Forward();
			//myAnimationComponentController->Walk();
			break;
		case EInputEvent::MoveBackward:
			//myMovement = -myCamera->GameObject().myTransform->GetLocalMatrix().Backward();
			//myAnimationComponentController->Walk();
			break;
		case EInputEvent::MoveLeft:
			//myMovement = myCamera->GameObject().myTransform->GetLocalMatrix().Left();
			//myAnimationComponentController->Walk();
			break;
		case EInputEvent::MoveRight:
			//myMovement = myCamera->GameObject().myTransform->GetLocalMatrix().Right();
			//myAnimationComponentController->Walk();
			break;
		case EInputEvent::Jump:
			if (myIsGrounded == true)
			{
				myHasJumped = true;
				myIsJumping = true;
				myIsGrounded = false;
			}
			break;
		case EInputEvent::Crouch:
			Crouch();
			break;

		case EInputEvent::Pull:
		{
			myAnimationComponentController->Pull(gPretendObjectCurrentDistance, gPretendObjectDistanceFromPlayer);
			gPretendObjectCurrentDistance = min(gPretendObjectCurrentDistance + CTimer::Dt() * 24.0f, gPretendObjectDistanceFromPlayer);
		}break;

		case EInputEvent::Push:
			myAnimationComponentController->Push();
			gPretendObjectCurrentDistance = 0.0f;
			break;

		case EInputEvent::ResetEntities:
			myController->SetPosition(myRespawnPosition);
			GameObject().myTransform->Position(myController->GetPosition());
			break;

		case EInputEvent::SetResetPointEntities:
			myRespawnPosition = myController->GetPosition();
			break;

		default:break;
	}

	myMovement.y = y;

	if (myLadderHasTriggered)
	{
		myMovement.y = myMovement.z;
		std::cout << myMovement.z << std::endl;
		myMovement.z = 0.0f;
		//myMovement = { 0.f, myMovement.y,0.f };
		//Move(myMovement * mySpeed);
	}
	else
	{
		//Move(myMovement * mySpeed);
	}
	//myMovement.y = 0.f;
	//myMovement = { 0.f, myMovement.y,0.f };
}

void CPlayerControllerComponent::ControllerUpdate()
{
	/*DirectX::SimpleMath::Vector3 input(0, 0, 0);
	DirectX::SimpleMath::Vector3 forward(0, 0, 0);
	DirectX::SimpleMath::Vector3 right(0, 0, 0);
	input.z += Input::GetInstance()->IsKeyDown('W') ? 1.f : 0.f;
	input.z += Input::GetInstance()->IsKeyDown('S') ? -1.f : 0.f;
	
	input.x += Input::GetInstance()->IsKeyDown('D') ? 1.f : 0.f;
	input.x += Input::GetInstance()->IsKeyDown('A') ? -1.f : 0.f;
	myAnimationComponentController->Walk();

	Vector3 dir = -myCamera->GameObject().myTransform->GetLocalMatrix().Right();
	Vector3 cross = dir.Cross(input);
	cross.Cross(myCamera->GameObject().myTransform->GetLocalMatrix().Up());
	myMovement = cross;*/
	//std::cout << "X: " << dir.x << " Z " << dir.y << std::endl;
	//std::cout << "X: " << cross.x << "Y: " << cross.y << "Z: " << cross.z << std::endl;
	Vector3 horizontal =	-GameObject().myTransform->GetLocalMatrix().Right() * Input::GetInstance()->GetAxis(Input::EAxis::Horizontal);
	Vector3 vertical =	-GameObject().myTransform->GetLocalMatrix().Forward() * Input::GetInstance()->GetAxis(Input::EAxis::Vertical);
	float y = myMovement.y;
	myMovement = (horizontal + vertical) /* * mySpeed*/;
	myMovement.y = y;
	//myMovement.y = Input::GetInstance()->IsKeyDown('E') ? myCameraMoveSpeed * verticalMoveSpeedModifier : cameraMovementInput.y;
	//myMovement.y = Input::GetInstance()->IsKeyDown('Q') ? -myCameraMoveSpeed * verticalMoveSpeedModifier : cameraMovementInput.y;
	//Vector3 dir = myController->GetPosition() - myCamera->GameObject().myTransform->GetLocalMatrix().Forward() * aInput.z;
	//dir = dir + myCamera->GameObject().myTransform->GetLocalMatrix().Right() * aInput.x;
}

void CPlayerControllerComponent::Move(Vector3 aDir)
{
	//std::cout << "Gravity: " << myMovement.y << std::endl;
	
	//ir.x = aInput.x * -myCamera->GameObject().myTransform->GetLocalMatrix().Right();
	physx::PxControllerCollisionFlags collisionflag = myController->GetController().move({ aDir.x, aDir.y, aDir.z}, 0, CTimer::FixedDt(), 0);
	
	if (collisionflag != physx::PxControllerCollisionFlag::eCOLLISION_DOWN )
	{
		myIsGrounded = false;

	}

	if (collisionflag == physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
	{
		myIsGrounded = true;
		if(aDir.x != 0.0f || aDir.z != 0.0f)
			myAnimationComponentController->Walk();
	}

}

void CPlayerControllerComponent::SetControllerPosition(const Vector3& aPos)
{
	myController->GetController().setPosition({ aPos.x, aPos.y, aPos.x });
}

void CPlayerControllerComponent::Crouch()
{
	myIsCrouching = !myIsCrouching;
	if (myIsCrouching)
	{
		myController->GetController().resize(myColliderHeightCrouched);
		GameObject().myTransform->FetchChildren()[0]->Position({ 0.0f, myCameraPosYCrouching, myCameraPosZ });// Equivalent to myCamera->GameObject().myTransform->Position
		mySpeed = myCrouchSpeed;
		// THIS IS TEMP :)
		myAnimationComponentController->TakeDamage();// TEMP :)
		// SUPER TEMP :)
	}
	else
	{
		myController->GetController().resize(myColliderHeightStanding);
		GameObject().myTransform->FetchChildren()[0]->Position({ 0.0f, myCameraPosYStanding, myCameraPosZ });// Equivalent to myCamera->GameObject().myTransform->Position
		mySpeed = myWalkSpeed;
	}
}

void CPlayerControllerComponent::ResetPlayerPosition()
{
	myController->SetPosition(myRespawnPosition);
}

CCharacterController* CPlayerControllerComponent::GetCharacterController()
{
	return myController;
}

const Vector3 CPlayerControllerComponent::GetLinearVelocity()
{
	const PxVec3 pxVec3 = myController->GetController().getActor()->getLinearVelocity();
	//const Vector3& vec3 = {pxVec3.x, pxVec3.y, pxVec3.z};
	return {pxVec3.x, pxVec3.y, pxVec3.z};
}

void CPlayerControllerComponent::LadderEnter()
{
	myLadderHasTriggered = true;
}

void CPlayerControllerComponent::LadderExit()
{
	myLadderHasTriggered = false;
}

void CPlayerControllerComponent::LadderUpdate()
{
	if (myLadderHasTriggered)
	{
		//Nuddar vi Marken?

		//Försöker vi gå neråt?
	}

	//Best�mmer n�r myIsOnladder s�tts till false

	//G�ra s� att vi g�r upp och ner f�r Ladder n�r vi trycker p� W eller S
	/*if (myIsGrounded && myMovement.LengthSquared() > 0.25f)
	{
		myIsOnLadder = false;
	}*/

	//if (Input::GetInstance()->IsKeyPressed('K'))
	//{
	//	myIsOnLadder = false;
	//}
}
