#include "stdafx.h"
#include "InGameState.h"

#include <GameObject.h>
#include <TransformComponent.h>
#include <ModelComponent.h>
#include <DecalComponent.h>
#include <PointLightComponent.h>

#include <Scene.h>
#include <Engine.h>
#include <PostMaster.h>
#include <MainSingleton.h>
#include <CollisionManager.h>
#include <PointLight.h>

#include <JsonReader.h>
#include <SceneManager.h>

#include <VFXSystemComponent.h>
#include <VFXMeshFactory.h>
#include <ParticleEmitterFactory.h>

#include <TextFactory.h>
#include <TextInstance.h>
#include "EnemyComponent.h"

void TEMP_VFX(CScene* aScene);
void TEMP_DeferredRenderingTests(CScene* aScene);

CInGameState::CInGameState(CStateStack& aStateStack, const CStateStack::EState aState)
	: CState(aStateStack, aState),
	myExitLevel(false)
{
}

CInGameState::~CInGameState() {}


void CInGameState::Awake()
{
	CJsonReader::Get()->InitFromGenerated();
	CScene* scene = CSceneManager::CreateEmpty();
	TEMP_VFX(scene);
	//CEnemyComponent::CreateEnemy(ASSETPATH("Assets/Graphics/Character/Enemy/CH_E_Robot_SK.fbx"), 2.0f, 10.0f);
	//scene->AddInstance(model);
	CEngine::GetInstance()->AddScene(myState, scene);
	CMainSingleton::PostMaster().Subscribe("LoadScene", this);
}


void CInGameState::Start()
{
	CEngine::GetInstance()->SetActiveScene(myState);
	TEMP_VFX(&CEngine::GetInstance()->GetActiveScene());
	IRONWROUGHT->GetActiveScene().CanvasIsHUD();

	myExitLevel = false;
}

void CInGameState::Stop()
{
	IRONWROUGHT->RemoveScene(myState);
	CMainSingleton::CollisionManager().ClearColliders();
}

void CInGameState::Update()
{

	IRONWROUGHT->GetActiveScene().UpdateCanvas();

	if (Input::GetInstance()->IsKeyPressed(VK_ESCAPE))
	{
		myStateStack.PushState(CStateStack::EState::PauseMenu);
	}

}

void CInGameState::ReceiveEvent(const EInputEvent aEvent)
{
	if (this == myStateStack.GetTop())
	{
		switch (aEvent)
		{
			case IInputObserver::EInputEvent::PauseGame:
				break;
			default:
				break;
		}
	}
}

void CInGameState::Receive(const SStringMessage& aMessage)
{
	const char* test = "LoadScene";
	if (aMessage.myMessageType == test)
	{
		Start();
	}
	myExitLevel = true;
}

void CInGameState::Receive(const SMessage& /*aMessage*/)
{
	//switch (aMessage.myMessageType)
	//{
	//	default:break;
	//}
}

void TEMP_DeferredRenderingTests(CScene* scene)
{
	CGameObject* chest = new CGameObject(1337);
	chest->AddComponent<CModelComponent>(*chest, std::string(ASSETPATH("Assets/Graphics/Exempel_Modeller/Wall/Wall.fbx")));
	chest->GetComponent<CTransformComponent>()->Position({5.0f,0.0f,5.0f});
	chest->myTransform->Rotation({ 0.0f,180.0f,0.0f });

	CGameObject* chest55 = new CGameObject(123123123);
	chest55->AddComponent<CModelComponent>(*chest55, std::string(ASSETPATH("Assets/Graphics/Exempel_Modeller/Wall/Wall.fbx")));
	chest55->GetComponent<CTransformComponent>()->Position({8.0f,0.0f,5.0f});
	chest55->myTransform->Rotation({ 0.0f,0.0f,0.0f });

	CGameObject* chest4 = new CGameObject(132342347);
	chest4->AddComponent<CModelComponent>(*chest4, std::string(ASSETPATH("Assets/Graphics/Exempel_Modeller/Wall/Wall.fbx")));
	chest4->GetComponent<CTransformComponent>()->Position({-1.0f,0.0f,5.0f});
	//chest4->myTransform->Scale({ 100.0f,100.0f,100.0f });
	chest4->myTransform->Rotation({ 0.0f,90.0f,0.0f });

	CGameObject* chest44 = new CGameObject(132342347);
	chest44->AddComponent<CModelComponent>(*chest44, std::string(ASSETPATH("Assets/Graphics/Exempel_Modeller/Wall/Wall.fbx")));
	chest44->GetComponent<CTransformComponent>()->Position({2.0f,-1.0f,-20.0f});
	chest44->myTransform->Rotation({ 90.0f,0.0f,0.0f });
	chest44->myTransform->Scale({ 50.0f,50.0f,50.0f });

	scene->AddInstance(chest);
	scene->AddInstance(chest4);
	scene->AddInstance(chest44);
	scene->AddInstance(chest55);
	//scene->AddInstance(chest2);
	//scene->AddInstance(chest3);

	constexpr int numPointLights = 0;
	std::vector<CGameObject*> pointLights;
	float x = -2.0f;
	float y = 1.0f;
	for (int i = 0; i < numPointLights; ++i)
	{
		if ((i + 1) % 10 == 0)
		{
			x = -2.0f;
			y += 1.0f;
		}
		x -= 1.0f;

		CGameObject* pl = new CGameObject(1789 + i);
		pl->AddComponent<CPointLightComponent>(*pl, 15.f, Vector3{ 1,1,1 }, 10.f);
		pl->myTransform->Position({ x, y, -3.0f });

		int thirdRange = numPointLights / 3;
		float r = (i < thirdRange ? 1.0f : 0.0f);
		float g = (i > thirdRange && i < thirdRange * 2 ? 1.0f : 0.0f);
		float b = (i > thirdRange * 2 && i < thirdRange * 3 ? 1.0f : 0.0f);
		pl->GetComponent<CPointLightComponent>()->GetPointLight()->SetColor({ r,g,b });
		pl->GetComponent<CPointLightComponent>()->GetPointLight()->SetPosition({ x, y, -3.0f });

		scene->AddInstance(pl->GetComponent<CPointLightComponent>()->GetPointLight());
		scene->AddInstance(pl);
	}
	for (int i = 0; i < 5; ++i)
	{
		CGameObject* pl = new CGameObject(9999 + i);
		pl->AddComponent<CPointLightComponent>(*pl, 10.f, Vector3{ 1,1,1 }, 10.f);
		pointLights.emplace_back(pl);
		pl->myTransform->Position({ 0xDEAD, 0xDEAD, 0xDEAD });

	}
	pointLights[0]->myTransform->Position({ 6.0f, 0.0f, -10.0f });
	pointLights[0]->GetComponent<CPointLightComponent>()->GetPointLight()->SetColor({ 0.0f,20.0f,0.0f });
	pointLights[0]->GetComponent<CPointLightComponent>()->GetPointLight()->SetPosition({ 0.0f,0.0f,-3.0f });

	pointLights[1]->myTransform->Position({ 4.0f, 1.0f, -10.0f });
	pointLights[1]->GetComponent<CPointLightComponent>()->GetPointLight()->SetColor({ 20.0f,0.0f,0.0f });
	pointLights[1]->GetComponent<CPointLightComponent>()->GetPointLight()->SetPosition({ 6.0f,0.0f,-1.0f });

	pointLights[2]->myTransform->Position({ 7.0f, 2.0f, -10.0f });
	pointLights[2]->GetComponent<CPointLightComponent>()->GetPointLight()->SetColor({ 0.0f,0.0f,20.0f });
	pointLights[2]->GetComponent<CPointLightComponent>()->GetPointLight()->SetPosition({ 12.0f,0.0f,-2.0f });

	pointLights[3]->myTransform->Position({ 6.0f, 0.0f, -10.0f });
	pointLights[3]->GetComponent<CPointLightComponent>()->GetPointLight()->SetColor({ 100.0f,100.0f,0.0f });
	pointLights[3]->GetComponent<CPointLightComponent>()->GetPointLight()->SetPosition({ 18.0f,2.0f,-2.0f });

	pointLights[4]->myTransform->Position({ 10.0f, 2.0f, -10.0f });
	pointLights[4]->GetComponent<CPointLightComponent>()->GetPointLight()->SetColor({ 0.5f,50.0f,50.0f });
	pointLights[4]->GetComponent<CPointLightComponent>()->GetPointLight()->SetPosition({ 24.0f,1.0f,-2.0f });

	scene->AddInstance(pointLights[0]->GetComponent<CPointLightComponent>()->GetPointLight());
	scene->AddInstance(pointLights[1]->GetComponent<CPointLightComponent>()->GetPointLight());
	scene->AddInstance(pointLights[2]->GetComponent<CPointLightComponent>()->GetPointLight());
	scene->AddInstance(pointLights[3]->GetComponent<CPointLightComponent>()->GetPointLight());
	scene->AddInstance(pointLights[4]->GetComponent<CPointLightComponent>()->GetPointLight());

	CTextInstance* text = new CTextInstance();
	text->Init(CTextFactory::GetInstance()->GetText("Text/baskerville16"));
	text->SetText("hello");
	text->SetPosition({0.0f, 0.0f});
	scene->AddInstance(text);
}

void CInGameState::TEMP_DecalTests(CScene* aScene)
{
	CGameObject* chest = new CGameObject(1337);
	chest->AddComponent<CModelComponent>(*chest, std::string(ASSETPATH("Assets/Graphics/Exempel_Modeller/Wall/Wall.fbx")));
	chest->GetComponent<CTransformComponent>()->Position({ 0.0f,0.0f,0.0f });
	chest->myTransform->Rotation({ 0.0f,0,0.0f });
	aScene->AddInstance(chest);

	//CGameObject* foliage = new CGameObject(13330);
	//foliage->AddComponent<CModelComponent>(*foliage, std::string(ASSETPATH("Assets/Graphics/Environmentprops/Static props/Foliage_test.fbx")));
	//foliage->GetComponent<CTransformComponent>()->Position({ 0.0f,0.0f,-1.0f });
	//foliage->myTransform->Rotation({ 0.0f,0,0.0f });
	//aScene->AddInstance(foliage);

	CGameObject* decal = new CGameObject(20000);
	decal->AddComponent<CDecalComponent>(*decal, "gradient");
	decal->GetComponent<CDecalComponent>()->SetAlphaThreshold(0.3f);
	decal->myTransform->Position({ 0.0f, 1.0f, 0.0f });
	decal->myTransform->Scale({ 1.0f, 1.0f, 1.0f });
	myDecal = decal;
	aScene->AddInstance(decal);

	//decal->GetComponent<CTransformComponent>()->Position({ 29.0f,2.0f, 0.0f });
	//decal->myTransform->Rotation({ 90.0f, 0.0f, 0.0f });
	//decal->GetComponent<CTransformComponent>()->Position({ 33.0f,2.0f, 25.5f });
	//decal->GetComponent<CTransformComponent>()->Position({ 14.0f, 1.0f, 20.0f });
	//decal->myTransform->Rotation({ 0.0f, 0.0f, 0.0f });

	//CGameObject* decal2 = new CGameObject(20001);
	//decal2->AddComponent<CDecalComponent>(*decal2, "Alpha");
	//decal2->GetComponent<CTransformComponent>()->Position({ 12.0f, 1.5f, 20.0f });
	//decal2->myTransform->Rotation({ 0.0f, 45.0f, 0.0f });
	//decal2->myTransform->Scale({ 1.0f, 1.0f, 1.0f });
	//aScene->AddInstance(decal2);
}

void TEMP_VFX(CScene* aScene)
{
	static int id = 500;
	CGameObject* abilityObject = new CGameObject(id++);
	abilityObject->AddComponent<CVFXSystemComponent>(*abilityObject, ASSETPATH("Assets/Graphics/VFX/JSON/VFXSystem_ToLoad.json"));

	//abilityObject->Awake();
	//abilityObject->Start();

	aScene->AddInstance(abilityObject);
	aScene->SetVFXTester(abilityObject);
}
