#include "stdafx.h"
#include "NodeTypeGameObjectSetRotation.h"
#include "NodeInstance.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "Engine.h"
#include "Scene.h"
#include "GraphManager.h"

CNodeTypeGameObjectSetRotation::CNodeTypeGameObjectSetRotation()
{
	myPins.push_back(SPin("IN", SPin::EPinTypeInOut::EPinTypeInOut_IN, SPin::EPinType::EFlow));	//0
	myPins.push_back(SPin("OUT", SPin::EPinTypeInOut::EPinTypeInOut_OUT));						//1
	myPins.push_back(SPin("Rot", SPin::EPinTypeInOut::EPinTypeInOut_IN, SPin::EPinType::EVector3));	//2
	//myPins.push_back(SPin("Yaw", SPin::EPinTypeInOut::EPinTypeInOut_IN, SPin::EPinType::EFloat));	//3
	//myPins.push_back(SPin("Roll", SPin::EPinTypeInOut::EPinTypeInOut_IN, SPin::EPinType::EFloat));	//4
	DeclareDataOnPinIfNecessary<Vector3>(myPins[2], Vector3(0.f, 0.f, 0.f));
}

int CNodeTypeGameObjectSetRotation::OnEnter(CNodeInstance* aTriggeringNodeInstance)
{
	CGameObject* gameObject = IRONWROUGHT_ACTIVE_SCENE.FindObjectWithID(aTriggeringNodeInstance->GraphManager()->GetCurrentBlueprintInstanceID());

	SPin::EPinType outType;
	NodeDataPtr someData = nullptr;
	size_t outSize = 0;

	GetDataOnPin(aTriggeringNodeInstance, 2, outType, someData, outSize);
	Vector3 newRotation = NodeData::Get<Vector3>(someData);

	//GetDataOnPin(aTriggeringNodeInstance, 3, outType, someData, outSize);
	//float y = NodeData::Get<float>(someData);

	//GetDataOnPin(aTriggeringNodeInstance, 4, outType, someData, outSize);
	//float z = NodeData::Get<float>(someData);

	//Vector3 newRotation = { x, y, z };
	gameObject->myTransform->Rotation(newRotation);

	return 1;
}
