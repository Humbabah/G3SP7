#include "stdafx.h"
#include "NodeType.h"
#include "NodeTypeStartDefault.h"
#include "NodeTypeStartKeyboardInput.h"
#include "NodeTypeDebugPrint.h"
#include "NodeInstance.h"
#include "NodeTypeGameObjectGetPosition.h"
#include "NodeTypeGameObjectSetPosition.h"
#include "NodeTypeGameObjectMove.h"
#include "NodeTypeMathAdd.h"
#include "NodeTypeMathSub.h"
#include "NodeTypeMathMul.h"
#include "NodeTypeMathCos.h"
#include "NodeTypeMathSin.h"
#include "NodeTypeMathAbs.h"
#include "NodeTypeMathAtan2.h"
#include "NodeTypeMathCeil.h"
#include "NodeTypeMathFloor.h"
#include "NodeTypeMathToRadians.h"
#include "NodeTypeMathToDegrees.h"
#include "NodeTypeMathDot.h"
#include "NodeTypeMathLerp.h"
#include "NodeTypeMathSaturate.h"
#include "NodeTypeMathSmoothstep.h"
#include "NodeTypeMathMax.h"
#include "NodeTypeMathMin.h"
#include "NodeTypeMathDistance.h"
#include "NodeTypeInputGetMousePosition.h"
#include "NodeTypeTimeTimer.h"
#include "NodeTypeTimeDeltaTotal.h"
#include "NodeTypeActionPrint.h"
#include "NodeTypeDecisionFloat.h"
#include "NodeTypeDecisionTreeRun.h"
#include "NodeTypeVariableSetFloat.h"
#include "NodeTypeVariableSetInt.h"
#include "NodeTypeVariableSetBool.h"
#include "NodeTypeVariableGetFloat.h"
#include "NodeTypeVariableGetInt.h"
#include "NodeTypeVariableGetBool.h"

//#include "NodeData.h"

CNodeType* CNodeTypeCollector::myTypes[128];
unsigned short CNodeTypeCollector::myTypeCounter = 0;
unsigned short CNodeTypeCollector::myTypeCount = 0;

std::vector<unsigned int> UID::myAllUIDs;
unsigned int UID::myGlobalUID = 0;

void CNodeTypeCollector::PopulateTypes()
{
	RegisterType<CNodeTypeStartDefault>("Default Start");
	RegisterType<CNodeTypeStartKeyboardInput>("Keyboard Input Start");
	RegisterType<CNodeTypeDebugPrint>("Print Message");
	RegisterType<CNodeTypeMathAdd>("Add");
	RegisterType<CNodeTypeMathSub>("Sub");
	RegisterType<CNodeTypeMathMul>("Mul");
	RegisterType<CNodeTypeMathCos>("Cos");
	RegisterType<CNodeTypeMathSin>("Sin");
	RegisterType<CNodeTypeMathAbs>("Abs");
	RegisterType<CNodeTypeMathAtan2>("Atan2");
	RegisterType<CNodeTypeMathCeil>("Ceil");
	RegisterType<CNodeTypeMathFloor>("Floor");
	RegisterType<CNodeTypeMathToRadians>("To Radians");
	RegisterType<CNodeTypeMathToDegrees>("To Degrees");
	RegisterType<CNodeTypeMathDot>("Dot");
	RegisterType<CNodeTypeMathLerp>("Lerp");
	RegisterType<CNodeTypeMathSaturate>("Saturate");
	RegisterType<CNodeTypeMathSmoothstep>("Smoothstep");
	RegisterType<CNodeTypeMathMax>("Max");
	RegisterType<CNodeTypeMathMin>("Min");
	RegisterType<CNodeTypeMathDistance>("Distance");
	RegisterType<CNodeTypeGameObjectGetPosition>("Get Object Position");
	RegisterType<CNodeTypeGameObjectSetPosition>("Set Object Position");
	RegisterType<CNodeTypeGameObjectMove>("Move Object");
	RegisterType<CNodeTypeInputGetMousePosition>("Mouse Position");
	RegisterType<CNodeTypeActionPrint>("Print Action");
	RegisterType<CNodeTypeDecisionFloat>("Float Decision");
	RegisterType<CNodeTypeDecisionTreeRun>("Run Tree");
	RegisterType<CNodeTypeTimeDeltaTotal>("Delta & Total Time");
	RegisterType<CNodeTypeTimeTimer>("Countdown Timer");
	//RegisterType<CNodeTypeVariableFloat>("Float");
	//RegisterType<CNodeTypeVariableInt>("Int");
	//RegisterType<CNodeTypeVariableBool>("Bool");
}

void CNodeTypeCollector::RegisterNewDataType(std::string aNodeName, unsigned int aType)
{
	switch (aType)
	{
	case 0:
	{
		RegisterDataType<CNodeTypeVariableSetFloat>("Set: " + aNodeName, aNodeName);
		RegisterDataType<CNodeTypeVariableGetFloat>("Get: " + aNodeName, aNodeName);
	}
		break;
	case 1:
	{
		RegisterDataType<CNodeTypeVariableSetInt>("Set: " + aNodeName, aNodeName);
		RegisterDataType<CNodeTypeVariableGetInt>("Get: " + aNodeName, aNodeName);
	}
		break;
	case 2:
	{
		RegisterDataType<CNodeTypeVariableSetBool>("Set: " + aNodeName, aNodeName);
		RegisterDataType<CNodeTypeVariableGetBool>("Get: " + aNodeName, aNodeName);
	}
		break;
	default:
		break;
	}
}

void CNodeType::ClearNodeInstanceFromMap(CNodeInstance* /*aTriggeringNodeInstance*/)
{
}

int CNodeType::DoEnter(CNodeInstance* aTriggeringNodeInstance)
{
	int index = OnEnter(aTriggeringNodeInstance);
	aTriggeringNodeInstance->myEnteredTimer = 3.0f;
	return index;
}

std::vector<SPin> CNodeType::GetPins()
{
	return myPins;
}

void CNodeType::GetDataOnPin(CNodeInstance* aTriggeringNodeInstance, unsigned int aPinIndex, SPin::EPinType& outType, NodeDataPtr& someData, size_t& outSize)
{
	aTriggeringNodeInstance->FetchData(outType, someData, outSize, aPinIndex);
}
