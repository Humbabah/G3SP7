#pragma once
#include "CNodeType.h"
#include "NodeTypes.h"

class CNodeTypeMathLerp : public CNodeType
{
public:
	CNodeTypeMathLerp();
	int OnEnter(class CNodeInstance* aTriggeringNodeInstance) override;
	virtual std::string GetNodeName() { return "Lerp"; }
	std::string GetNodeTypeCategory() override { return "Math"; }
};

