#include "stdafx.h"
#include "NodeTypeGameObjectMoveToPosition.h"
#include "NodeInstance.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "Timer.h"

CNodeTypeGameObjectMoveToPosition::CNodeTypeGameObjectMoveToPosition()
{
	myPins.push_back(SPin("IN", SPin::EPinTypeInOut::EPinTypeInOut_IN, SPin::EPinType::EFlow));			//0
	myPins.push_back(SPin("OUT", SPin::EPinTypeInOut::EPinTypeInOut_OUT));								//1
	myPins.push_back(SPin("X Pos", SPin::EPinTypeInOut::EPinTypeInOut_IN, SPin::EPinType::EFloat));		//2
	myPins.push_back(SPin("Is at Pos", SPin::EPinTypeInOut::EPinTypeInOut_OUT, SPin::EPinType::EBool));	//3
	myPins.push_back(SPin("Y Pos", SPin::EPinTypeInOut::EPinTypeInOut_IN, SPin::EPinType::EFloat));		//4
	myPins.push_back(SPin("Z Pos", SPin::EPinTypeInOut::EPinTypeInOut_IN, SPin::EPinType::EFloat));		//5
	myPins.push_back(SPin("Speed", SPin::EPinTypeInOut::EPinTypeInOut_IN, SPin::EPinType::EFloat));		//6
}

int CNodeTypeGameObjectMoveToPosition::OnEnter(CNodeInstance* aTriggeringNodeInstance)
{
	CGameObject* gameObject = aTriggeringNodeInstance->GetCurrentGameObject()[1];

	SPin::EPinType outType;
	NodeDataPtr someData = nullptr;
	size_t outSize = 0;

	bool stopMovingX = false;
	bool stopMovingY = false;
	bool stopMovingZ = false;
	bool stopMoving = false;

	GetDataOnPin(aTriggeringNodeInstance, 2, outType, someData, outSize);
	float x = NodeData::Get<float>(someData);

	GetDataOnPin(aTriggeringNodeInstance, 4, outType, someData, outSize);
	float y = NodeData::Get<float>(someData);

	GetDataOnPin(aTriggeringNodeInstance, 5, outType, someData, outSize);
	float z = NodeData::Get<float>(someData);

	GetDataOnPin(aTriggeringNodeInstance, 6, outType, someData, outSize);
	float speed = NodeData::Get<float>(someData);

	Vector3 currentPosition = aTriggeringNodeInstance->GetCurrentGameObject()[1]->myTransform->Position();
	Vector3 direction = { (x - currentPosition.x), (y - currentPosition.y), (z - currentPosition.z) };

	if ((direction.x < 0.01f && direction.x > -0.01f))
	{
		stopMovingX = true;
		direction.x = 0.0f;
	}
	if ((direction.y <  0.01f && direction.y > -0.01f))
	{
		stopMovingY = true;
		direction.y = 0.0f;
	}
	if ((direction.z < 0.01f && direction.z > -0.01f))
	{
		stopMovingZ = true;
		direction.z = 0.0f;
	}

	if (stopMovingX && stopMovingY && stopMovingZ)
	{
		stopMoving = true;
	}

	if (!stopMoving)
	{
		direction.Normalize();
		direction *= CTimer::Dt() * speed;

		gameObject->myTransform->Move({ direction.x,  direction.y, direction.z });
	}


	std::vector<SPin>& pins = aTriggeringNodeInstance->GetPins();
	DeclareDataOnPinIfNecessary<bool>(pins[0]);
	memcpy(pins[0].myData, &stopMoving, sizeof(bool));

	return 1;
}
