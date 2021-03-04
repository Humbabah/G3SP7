#include "stdafx.h"
#include "NodeTypeVariableGetBool.h"
#include "NodeInstance.h"
#include "NodeDataManager.h"

CNodeTypeVariableGetBool::CNodeTypeVariableGetBool()
{
	myPins.push_back(SPin("OUT", SPin::EPinTypeInOut::EPinTypeInOut_OUT, SPin::EPinType::EBool));
}

int CNodeTypeVariableGetBool::OnEnter(CNodeInstance* aTriggeringNodeInstance)
{
	bool output = CNodeDataManager::Get()->GetData<bool>(myNodeDataKey);

	std::vector<SPin>& pins = aTriggeringNodeInstance->GetPins();
	DeclareDataOnPinIfNecessary<bool>(pins[0]);
	memcpy(pins[0].myData, &output, sizeof(bool));

	return -1;
}