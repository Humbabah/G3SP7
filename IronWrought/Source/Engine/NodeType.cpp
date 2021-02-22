#include "stdafx.h"
#include "NodeType.h"
#include "NodeTypeDefaultStart.h"
#include "NodeTypePrintMessage.h"
#include "NodeInstance.h"
#include "NodeTypeMathAdd.h"
#include "NodeTypeMathSub.h"
#include "NodeTypeMathMul.h"
#include "NodeTypeGetGameObjectPosition.h"
#include "NodeTypeSetGameObjectPosition.h"
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
#include "NodeTypeMathStorage.h"
#include "NodeTypeMathSmoothstep.h"
#include "NodeTypeMathMax.h"
#include "NodeTypeMathMin.h"
#include "NodeTypeMathDistance.h"
#include "NodeTypeGetMousePosition.h"
#include "NodeTypeForEachGameObject.h"
#include "NodeTypeVolumeTrigger.h"
#include "NodeTypeTimer.h"
#include "NodeTypePrintAction.h"
#include "NodeTypeFloatDecision.h"
#include "NodeTypeRunDecisionTree.h"
#include "NodeTypeDeltaTime.h"
#include "NodeTypeFloat.h"
#include "NodeTypeInt.h"
#include "NodeTypeBool.h"
#include "NodeTypeString.h"

CNodeType* CNodeTypeCollector::myTypes[128];
unsigned short CNodeTypeCollector::myTypeCounter = 0;
unsigned short CNodeTypeCollector::myTypeCount = 0;

std::vector<unsigned int> UID::myAllUIDs;
unsigned int UID::myGlobalUID = 0;

void CNodeTypeCollector::PopulateTypes()
{
	RegisterType<CNodeTypeDefaultStart>();
	RegisterType<CNodeTypePrintMessage>();
	RegisterType<CNodeTypeMathAdd>();
	RegisterType<CNodeTypeMathSub>();
	RegisterType<CNodeTypeMathMul>();
	RegisterType<CNodeTypeGetGameObjectPosition>();
	RegisterType<CNodeTypeSetGameObjectPosition>();
	RegisterType<CNodeTypeMathCos>();
	RegisterType<CNodeTypeMathSin>();
	RegisterType<CNodeTypeMathAbs>();
	RegisterType<CNodeTypeMathAtan2>();
	RegisterType<CNodeTypeMathCeil>();
	RegisterType<CNodeTypeMathFloor>();
	RegisterType<CNodeTypeMathToRadians>();
	RegisterType<CNodeTypeMathToDegrees>();
	RegisterType<CNodeTypeMathDot>();
	RegisterType<CNodeTypeMathLerp>();
	RegisterType<CNodeTypeMathSaturate>();
	RegisterType<CNodeTypeMathStorage>();
	RegisterType<CNodeTypeMathSmoothstep>();
	RegisterType<CNodeTypeMathMax>();
	RegisterType<CNodeTypeMathMin>();
	RegisterType<CNodeTypeMathDistance>();
	RegisterType<CNodeTypeGetMousePosition>();
	RegisterType<CNodeTypeForEachGameObject>();
	RegisterType<CNodeTypeVolumeTrigger>();
	RegisterType<CNodeTypeTimer>();
	RegisterType<CNodeTypePrintAction>();
	RegisterType<CNodeTypeFloatDecision>();
	RegisterType<CNodeTypeRunDecisionTree>();
	RegisterType<CNodeTypeDeltaTime>();
	RegisterType<CNodeTypeFloat>();
	RegisterType<CNodeTypeInt>();
	RegisterType<CNodeTypeBool>();
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
