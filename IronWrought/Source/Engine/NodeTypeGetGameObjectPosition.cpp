#include "stdafx.h"
#include "NodeTypeGetGameObjectPosition.h"
#include "NodeInstance.h"
#include "Scene.h"
#include "GameObject.h"
#include "GraphManager.h"

CNodeTypeGetGameObjectPosition::CNodeTypeGetGameObjectPosition()
{
    myPins.push_back(SPin("X", SPin::EPinTypeInOut::PinTypeInOut_OUT, SPin::EPinType::Float)); //0
    myPins.push_back(SPin("Y", SPin::EPinTypeInOut::PinTypeInOut_OUT, SPin::EPinType::Float)); //1
	myPins.push_back(SPin("Z", SPin::EPinTypeInOut::PinTypeInOut_OUT, SPin::EPinType::Float)); //2
}

int CNodeTypeGetGameObjectPosition::OnEnter(CNodeInstance* aTriggeringNodeInstance)
{
	CGameObject* gameObject = aTriggeringNodeInstance->GetCurrentGameObject();
	Vector3 position = gameObject->myTransform->Position();

	std::vector<SPin>& pins = aTriggeringNodeInstance->GetPins();
	DeclareDataOnPinIfNecessary<float>(pins[0]);
	memcpy(pins[0].myData, &position.x, sizeof(float));
	DeclareDataOnPinIfNecessary<float>(pins[1]);
	memcpy(pins[1].myData, &position.y, sizeof(float));
	DeclareDataOnPinIfNecessary<float>(pins[2]);
	memcpy(pins[2].myData, &position.z, sizeof(float));

	return -1;
}


//SPin::EPinType outType;
//NodeDataPtr someData = nullptr;
//size_t outSize = 0;

//GetDataOnPin(aTriggeringNodeInstance, 0, outType, someData, outSize);
//int input1 = NodeData::Get<int>(someData);

//Tga2D::Vector2f position = CScene::GetActiveScene()->GetPositionFromID(input1);