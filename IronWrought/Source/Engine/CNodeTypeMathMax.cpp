#include "stdafx.h"
#include "CNodeTypeMathMax.h"
#include "CNodeInstance.h"

CNodeTypeMathMax::CNodeTypeMathMax()
{
	myPins.push_back(SPin("Val1", SPin::PinTypeInOut::PinTypeInOut_IN, SPin::PinType::Float)); // Pin index 0
	myPins.push_back(SPin("Val2", SPin::PinTypeInOut::PinTypeInOut_IN, SPin::PinType::Float)); // Pin index 1
	myPins.push_back(SPin("OUT", SPin::PinTypeInOut::PinTypeInOut_OUT, SPin::PinType::Float)); // Pin index 2
}

int CNodeTypeMathMax::OnEnter(class CNodeInstance* aTriggeringNodeInstance)
{
	// Three variables we can fill with data when we aquire data from another node
	SPin::PinType outType;
	NodeDataPtr someData = nullptr;
	size_t outSize = 0;
	// End

	GetDataOnPin(aTriggeringNodeInstance, 0, outType, someData, outSize); // Get data on pin index 0, this index is relative to what you push in the constructor
	float input1 = NodeData::Get<float>(someData); // Helper function to get a float from a void pointer, we know its a float as declared on the pin in our constructor

	GetDataOnPin(aTriggeringNodeInstance, 1, outType, someData, outSize); // Get data on pin index 1, this index is relative to what you push in the constructor
	float input2 = NodeData::Get<float>(someData); // Helper function to get a float from a void pointer, we know its a float as declared on the pin in our constructor

	float temp = (input1 > input2) ? input1 : input2;

	std::vector<SPin>& pins = aTriggeringNodeInstance->GetPins();
	DeclareDataOnPinIfNecessary<float>(pins[2]); // We need to create the memory so we can set some data, all pins have NO memory as default
	memcpy(pins[2].myData, &temp, sizeof(float)); // set the memory to the pin

	return -1; // If the node doesn't have a flow, return -1 see the print node for reference
}

