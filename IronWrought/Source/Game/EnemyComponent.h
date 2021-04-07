#pragma once
#include "Component.h"

class CAIController;
class CCharacterController;
class CGameObject;

struct SEnemySetting {
	float mySpeed;	//= 10.0f;
	float myDistance;//= 0.0f;
	float myRadius;	//= 0.0015f;
};

class CEnemyComponent : public CComponent
{
public:
	enum class EBehaviour {
		Patrol,
		Seek,
		Attack
	};

public:
	CEnemyComponent(CGameObject& aParent, const SEnemySetting& someSettings);
	~CEnemyComponent() override;

	void Awake() override;
	void Start() override;
	void Update()override;
	void SetState(EBehaviour aState);
	const EBehaviour GetState()const;
private:
	CCharacterController* myController;
	std::vector<CAIController*> myBehaviours;
	EBehaviour myCurrentState;
	CGameObject* myEnemy;
	CGameObject* myPlayer;
	SEnemySetting mySettings;
	
};

