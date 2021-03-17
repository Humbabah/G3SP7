#include "stdafx.h"
#include "TransformComponent.h"

using namespace DirectX::SimpleMath;

CTransformComponent::CTransformComponent(CGameObject& aParent, DirectX::SimpleMath::Vector3 aPosition)
	:  myScale(1.0f), 
	myParent(nullptr),
	CComponent(aParent)
{
	Scale({ 1.0f, 1.0f, 1.0f });
	Position(aPosition);
}

CTransformComponent::~CTransformComponent()
{
}

void CTransformComponent::Awake()
{
}

void CTransformComponent::Start()
{
	
}

void CTransformComponent::Update()
{

}

void CTransformComponent::LateUpdate()
{
	if (myParent != nullptr)
		myWorldTransform = DirectX::XMMatrixMultiply(myLocalTransform, myParent->myWorldTransform);		
	else
		myWorldTransform = myLocalTransform;
}

void CTransformComponent::Position(DirectX::SimpleMath::Vector3 aPosition)
{
	myParentOffset = aPosition;
	myLocalTransform.Translation(aPosition);
}

DirectX::SimpleMath::Vector3 CTransformComponent::Position() const
{
	return myLocalTransform.Translation();
}

void CTransformComponent::Rotation(DirectX::SimpleMath::Vector3 aRotation)
{
	Vector3 tempTranslation = myLocalTransform.Translation();

	Matrix tempRotation = Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(aRotation.y),
		DirectX::XMConvertToRadians(aRotation.x),
		DirectX::XMConvertToRadians(aRotation.z)
	);

	myLocalTransform = tempRotation;
	myLocalTransform *= Matrix::CreateScale(myScale);
	myLocalTransform.Translation(tempTranslation);
	//myLocalRotation += aRotation;
}

void CTransformComponent::Rotation(DirectX::SimpleMath::Quaternion aQuaternion) 
{
	Vector3 tempTranslation = myLocalTransform.Translation();

	Matrix tempRotation = Matrix::CreateFromQuaternion(
		aQuaternion
	);
	myLocalTransform = tempRotation;
	myLocalTransform *= Matrix::CreateScale(myScale);
	myLocalTransform.Translation(tempTranslation);
}

DirectX::SimpleMath::Quaternion CTransformComponent::Rotation() const
{
	DirectX::SimpleMath::Vector3 translation;
	DirectX::SimpleMath::Vector3 scale;
	DirectX::SimpleMath::Quaternion quat;
	GetLocalMatrix().Decompose(scale, quat, translation);
	return quat;
}

void CTransformComponent::Scale(DirectX::SimpleMath::Vector3 aScale)
{
	myScale = aScale;
	ResetScale();
}

void CTransformComponent::Scale(float aScale)
{
	Scale({ aScale, aScale, aScale });
}

DirectX::SimpleMath::Vector3 CTransformComponent::Scale() const
{
	return myScale;
}

void CTransformComponent::SetOutlineScale()
{
	Vector3 scale;
	Vector3 translation;
	Quaternion rotation;
	myLocalTransform.Decompose(scale, rotation, translation);
	myLocalTransform = Matrix::CreateFromQuaternion(rotation);
	myLocalTransform *= Matrix::CreateScale(myScale * 1.02f);
	myLocalTransform.Translation(translation);
}

void CTransformComponent::ResetScale()
{
	Vector3 scale;
	Vector3 translation;
	Quaternion rotation;
	myLocalTransform.Decompose(scale, rotation, translation);
	myLocalTransform = Matrix::CreateFromQuaternion(rotation);
	if (myParent == nullptr) {
		myLocalTransform *= Matrix::CreateScale(myScale);
	}
	myLocalTransform.Translation(translation);
}

void CTransformComponent::Transform(DirectX::SimpleMath::Vector3 aPosition, DirectX::SimpleMath::Vector3 aRotation)
{
	Rotation(aRotation);
	Position(aPosition);
}
void CTransformComponent::Transform(DirectX::SimpleMath::Matrix& aTransform)
{
	myLocalTransform = aTransform;
}

DirectX::SimpleMath::Matrix& CTransformComponent::Transform()
{
	return myWorldTransform;
}

void CTransformComponent::Move(DirectX::SimpleMath::Vector3 aMovement)
{
	myLocalTransform.Translation(myLocalTransform.Translation() + aMovement);
}

void CTransformComponent::MoveLocal(DirectX::SimpleMath::Vector3 aMovement)
{
	myLocalTransform.Translation(myLocalTransform.Translation() + myLocalTransform.Right() * aMovement.x);
	myLocalTransform.Translation(myLocalTransform.Translation() + myLocalTransform.Up() * aMovement.y);
	myLocalTransform.Translation(myLocalTransform.Translation() - myLocalTransform.Forward() * aMovement.z);
}

void CTransformComponent::Rotate(DirectX::SimpleMath::Vector3 aRotation)
{
	Vector3 tempTranslation = myLocalTransform.Translation();
	Matrix tempRotation = Matrix::CreateFromYawPitchRoll(aRotation.y, aRotation.x, aRotation.z);
	myLocalTransform *= tempRotation;
	myLocalTransform.Translation(tempTranslation);
}

void CTransformComponent::Rotate(DirectX::SimpleMath::Quaternion aQuaternion)
{
	Vector3 tempTranslation = myLocalTransform.Translation();
	Matrix tempRotation = Matrix::CreateFromQuaternion(aQuaternion);
	myLocalTransform *= tempRotation;
	myLocalTransform.Translation(tempTranslation);
}

DirectX::SimpleMath::Matrix CTransformComponent::GetWorldMatrix() const
{
	return myWorldTransform;
}

DirectX::SimpleMath::Matrix CTransformComponent::GetLocalMatrix() const
{
	return myLocalTransform;
}

void CTransformComponent::SetParent(CTransformComponent* aParent)
{
	myLocalTransform = DirectX::XMMatrixMultiply(myLocalTransform, aParent->myWorldTransform.Invert());
	myParent = aParent;
}

void CTransformComponent::RemoveParent()
{
	myLocalTransform = DirectX::XMMatrixMultiply(myLocalTransform, myParent->myWorldTransform);
	Vector3 translation; Vector3 scale;  Quaternion quat;
	GetLocalMatrix().Decompose(scale, quat, translation);
	myParent = nullptr;
}
