#pragma once
#include "Behaviour.h"
//#include <GravityGloveEditor.h>

class CTransformComponent;
class CRigidBodyComponent;

struct SGravityGloveSettings {
	float myPushForce;
	float myDistanceToMaxLinearVelocity;

	float myMaxPushForce;
	float myMinPushForce;

	float myMaxPullForce;
	float myMinPullForce;

	float myMaxDistance;
	float myCurrentDistanceInverseLerp;
};

class CGravityGloveComponent : public CBehaviour
{
	//friend class IronWroughtImGui::CGravityGloveEditor;
public:
	CGravityGloveComponent(CGameObject& aParent, CTransformComponent* aGravitySlot);
	~CGravityGloveComponent() override;

	void Awake() override;
	void Start() override;
	void Update() override;

	void Pull();
	void Push();

	void OnEnable() override;
	void OnDisable() override;


	SGravityGloveSettings& GetSettings() { return mySettings; }
	//void SetSettings(const SGravityGloveSettings& someSettings) { mySettings = someSettings; }


	SGravityGloveSettings mySettings;
private:
	float InverseLerp(float a, float b, float t) const
	{
		if (a == b)
		{
			if (t < a)
			{
				return 0.f;
			}
			else
			{
				return 1.f;
			}
		}

		if (a > b)
		{
			float temp = b;
			b = a;
			a = temp;
		}

		return (t - a) / (b - a);
	}

	float Lerp(float a, float b, float t) const
	{
		//if (t < a)
		//{
		//	return 0.f;
		//}
		//else
		//{
		//	return 1.f;
		//}

		//if (a > b)
		//{
		//	float temp = b;
		//	b = a;
		//	a = temp;
		//}

		return (1.0f - t) * a + b * t;
		//return a + t * (b - a);
	}

	float Remap(const float iMin, const float iMax, const float oMin, const float oMax, const float v) const
	{
		float t = InverseLerp(iMin, iMax, v);
		return Lerp(oMin, oMax, t);
	}

	CTransformComponent* myGravitySlot;
	CRigidBodyComponent* myCurrentTarget;

};


