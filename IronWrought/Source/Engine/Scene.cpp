#include "stdafx.h"
#include "Scene.h"

#include <algorithm>
#include "..\Game\AIBehavior.h"

#include "Component.h"
#include "GameObject.h"
#include "TransformComponent.h"

#include "Model.h"
#include "ModelFactory.h"
#include "ModelComponent.h"
#include "InstancedModelComponent.h"

#include "SpriteInstance.h"
#include "AnimatedUIElement.h"
#include "TextInstance.h"
#include "VFXInstance.h"

#include "LineInstance.h"
#include "LineFactory.h"

#include "EnvironmentLight.h"
#include "PointLight.h"

#include "Camera.h"
#include "CameraComponent.h"

#include "CollisionManager.h"

#include "NavmeshLoader.h"

#include "Debug.h"
//SETUP START
CScene::CScene(const unsigned int aGameObjectCount)
	: myIsReadyToRender(false)
	, myMainCamera(nullptr)
	, myEnvironmentLight(nullptr)
	, myNavMesh(nullptr)
	, myNavMeshGrid(nullptr)
	//, myEnemyBehavior(nullptr)
	//, myPlayer(nullptr)
	//, myBoss(nullptr)
{
	myGameObjects.reserve(aGameObjectCount);

	myModelsToOutline.resize(2);
	for (unsigned int i = 0; i < myModelsToOutline.size(); ++i)
	{
		myModelsToOutline[i] = nullptr;
	}

#ifdef _DEBUG
	myShouldRenderLineInstance = true;
	myGrid = new CLineInstance();
	myGrid->Init(CLineFactory::GetInstance()->CreateGrid({ 0.1f, 0.5f, 1.0f, 1.0f }));
	this->AddInstance(myGrid);
#endif
}

CScene::~CScene()
{
	myMainCamera = nullptr;
	delete myEnvironmentLight;
	myEnvironmentLight = nullptr;

	this->ClearGameObjects();
	this->ClearPointLights();
	this->ClearVFXInstances();
	// This must be deleted after gameobjects have let go of their pointer to it

	//if (myEnemyBehavior)
	//{
	//	delete myEnemyBehavior;
	//	myEnemyBehavior = nullptr;
	//}

#ifdef _DEBUG
	myGrid = nullptr;
	delete myGrid;
#endif
	if (myPXScene != nullptr) {
		myPXScene->release();
		myPXScene = nullptr;
	}

	// Any CScene that is not InGame's scene will not hold a NavMesh
	if (myNavMesh)
	{
		delete myNavMesh;
		myNavMesh = nullptr;
	}
	if (myNavMeshGrid)// -||-
	{
		delete myNavMeshGrid;
		myNavMeshGrid = nullptr;
	}
	// Even with this the memory still increases on every load!
}

bool CScene::Init()
{
	return true;
}

bool CScene::InitNavMesh(std::string aPath)
{
	CNavmeshLoader* loader = new CNavmeshLoader();
	myNavMesh = loader->LoadNavmesh(aPath);

	if (!myNavMesh)
	{
		return false;
	}

	std::vector<DirectX::SimpleMath::Vector3> positions;
	positions.resize(myNavMesh->myTriangles.size() * 6);

	for (UINT i = 0, j = 0; i < positions.size() && j < myNavMesh->myTriangles.size(); i += 6, j++)
	{
		positions[static_cast<int>(i + 0)] = myNavMesh->myTriangles[j]->myVertexPositions[0];
		positions[static_cast<int>(i + 1)] = myNavMesh->myTriangles[j]->myVertexPositions[1];
		positions[static_cast<int>(i + 2)] = myNavMesh->myTriangles[j]->myVertexPositions[2];
		positions[static_cast<int>(i + 3)] = myNavMesh->myTriangles[j]->myVertexPositions[0];
		positions[static_cast<int>(i + 4)] = myNavMesh->myTriangles[j]->myVertexPositions[1];
		positions[static_cast<int>(i + 5)] = myNavMesh->myTriangles[j]->myVertexPositions[2];
	}

	delete loader;
	loader = nullptr;
	return true;
}
//SETUP END
//SETTERS START
void CScene::MainCamera(CCameraComponent* aMainCamera)
{
	myMainCamera = aMainCamera;
}

bool CScene::EnvironmentLight(CEnvironmentLight* anEnvironmentLight)
{
	myEnvironmentLight = anEnvironmentLight;
	return true;
}

void CScene::ShouldRenderLineInstance(const bool aShouldRender)
{
#ifdef  _DEBUG
	myShouldRenderLineInstance = aShouldRender;
#else
	aShouldRender;
#endif //  _DEBUG
}
//SETTERS END
//GETTERS START
CCameraComponent* CScene::MainCamera()
{
	return myMainCamera;
}

CEnvironmentLight* CScene::EnvironmentLight()
{
	return myEnvironmentLight;
}

SNavMesh* CScene::NavMesh()
{
	return myNavMesh;
}

PxScene* CScene::PXScene()
{
	return myPXScene;
}

std::vector<CGameObject*> CScene::ModelsToOutline() const
{
	return myModelsToOutline;
}

std::vector<CPointLight*>& CScene::PointLights()
{
	return myPointLights;
}

std::vector<CTextInstance*> CScene::Texts()
{
	std::vector<CTextInstance*> textToRender;
	for (auto& text : myTexts) {
		if (text->GetShouldRender()) {
			textToRender.emplace_back(text);
		}
	}
	return textToRender;
}

const std::vector<CGameObject*>& CScene::ActiveGameObjects() const
{
	return myGameObjects;
}
//GETTERS END
//CULLING START
std::vector<CPointLight*> CScene::CullPointLights(CGameObject* /*aGameObject*/)
{
	//std::cout << __FUNCTION__ << " Reminde to add actual culling to this function!" << std::endl;
	return myPointLights;
}

std::pair<unsigned int, std::array<CPointLight*, LIGHTCOUNT>> CScene::CullLights(CGameObject* aGameObject)
{
	std::pair<unsigned int, std::array<CPointLight*, LIGHTCOUNT>> pointLightPair;
	UINT counter = 0;
	for (UINT i = 0; i < myPointLights.size(); ++i)
	{
		float distanceSquared = DirectX::SimpleMath::Vector3::DistanceSquared(myPointLights[i]->GetPosition(), aGameObject->GetComponent<CTransformComponent>()->Transform().Translation());
		float range = myPointLights[i]->GetRange();
		if (distanceSquared < (range * range))
		{
			pointLightPair.second[counter] = myPointLights[i];
			++counter;

			if (counter == LIGHTCOUNT)
			{
				break;
			}
		}
	}

	pointLightPair.first = counter;
	return pointLightPair;
}

const std::vector<CLineInstance*>& CScene::CullLineInstances() const
{
#ifdef _DEBUG
	if (myShouldRenderLineInstance)
		return myLineInstances;
	else
	{
		std::vector<CLineInstance*> temp;
		return std::move(temp);
	}
#else
	return myLineInstances;
#endif
}

const std::vector<SLineTime>& CScene::CullLines() const
{
	return CDebug::GetInstance()->GetLinesTime();
	//return CDebug::GetInstance()->GetLines();
}

std::vector<CVFXInstance*> CScene::CullVFX(CCameraComponent* /*aMainCamera*/)
{

	for (unsigned int i = 0; i < myVFXInstances.size(); ++i)
	{

		myVFXInstances[i]->Scroll({0.15f * CTimer::Dt(), 0.15f * CTimer::Dt()}, {0.15f * CTimer::Dt() , 0.15f * CTimer::Dt()});
	}
	return myVFXInstances;
}

std::vector<CAnimatedUIElement*> CScene::CullAnimatedUI(std::vector<CSpriteInstance*>& someFramesToReturn)
{
	std::vector<CAnimatedUIElement*> elementsToRender;
	for (auto& element : myAnimatedUIElements)
	{
		if (element->GetInstance()->GetShouldRender())
		{
			elementsToRender.emplace_back(element);
			someFramesToReturn.emplace_back(element->GetInstance());
		}
	}
	return elementsToRender;
}

LightPair CScene::CullLightInstanced(CInstancedModelComponent* aModelType)
{
	//S�tt s� att Range t�cker objektet l�ngst bort
	//if (myPlayer != nullptr) {
	//	aModelType->GameObject().myTransform->Position(GetPlayer()->myTransform->Position());
	//}

	std::pair<unsigned int, std::array<CPointLight*, LIGHTCOUNT>> pointLightPair;
	UINT counter = 0;
	for (UINT i = 0; i < myPointLights.size(); ++i)
	{
		float distanceSquared = DirectX::SimpleMath::Vector3::DistanceSquared(myPointLights[i]->GetPosition(), aModelType->GetComponent<CTransformComponent>()->Position());
		float range = myPointLights[i]->GetRange() * 200.0f;
		if (distanceSquared < (range * range))
		{
			pointLightPair.second[counter] = myPointLights[i];
			++counter;

			if (counter == LIGHTCOUNT)
			{
				break;
			}
		}
	}

	pointLightPair.first = counter;
	return pointLightPair;
}

std::vector<CGameObject*> CScene::CullGameObjects(CCameraComponent* /*aMainCamera*/)
{
	//NEED TO MAKE A PROPPER CULLING?
	return myGameObjects;
	//using namespace DirectX::SimpleMath;
	//Vector3 cameraPosition = aMainCamera->GameObject().myTransform->Transform().Translation();
	//std::vector<CGameObject*> culledGameObjects;
	//for (auto& gameObject : myGameObjects)
	//{
	//	if (!gameObject->Active())
	//	{
	//		continue;
	//	}
	//	if (gameObject->GetComponent<CInstancedModelComponent>())
	//	{
	//		culledGameObjects.emplace_back(gameObject);
	//		continue;
	//	}
	//
	//	float distanceToCameraSquared = Vector3::DistanceSquared(gameObject->GetComponent<CTransformComponent>()->Position(), cameraPosition);
	//	if (distanceToCameraSquared < 10000.0f)
	//	{
	//		culledGameObjects.emplace_back(gameObject);
	//	}
	//}
	//return culledGameObjects;
}

std::vector<CSpriteInstance*> CScene::CullSprites()
{
	std::vector<CSpriteInstance*> spritesToRender;

	for (UINT i = 0; i < mySpriteInstances.size(); ++i)
	{
		for (auto& sprite : mySpriteInstances[static_cast<ERenderOrder>(i)])
		{
			if (sprite->GetShouldRender())
			{
				spritesToRender.emplace_back(sprite);
			}
		}
	}

	return spritesToRender;
}
//CULLING END
//POPULATE SCENE START
bool CScene::AddInstance(CPointLight* aPointLight)
{
	myPointLights.emplace_back(aPointLight);
	return true;
}

bool CScene::AddInstance(CLineInstance* aLineInstance)
{
	myLineInstances.emplace_back(aLineInstance);
	return true;
}

bool CScene::AddInstance(CVFXInstance* aVFXInstance)
{
	myVFXInstances.emplace_back(aVFXInstance);
	return true;
}

bool CScene::AddInstance(CAnimatedUIElement* anAnimatedUIElement)
{
	if (!anAnimatedUIElement)
	{
		return false;
	}
	myAnimatedUIElements.emplace_back(anAnimatedUIElement);
	return true;
}

bool CScene::AddInstance(CTextInstance* aText)
{
	if (!aText)
	{
		return false;
	}
	myTexts.emplace_back(aText);
	return true;
}

bool CScene::AddInstance(CGameObject* aGameObject)
{
	myGameObjects.emplace_back(aGameObject);
	return true;
}

bool CScene::AddInstances(std::vector<CGameObject*>& someGameObjects)
{
	if (someGameObjects.size() == 0)
		return false;

	for (unsigned int i = 0; i < someGameObjects.size(); ++i)
	{
		myGameObjects.emplace_back(someGameObjects[i]);
	}


	//myGameObjects.insert(myGameObjects.end(), someGameObjects.begin(), someGameObjects.end());
	return true;
}

bool CScene::AddInstance(CSpriteInstance* aSprite)
{
	if (!aSprite)
	{
		return false;
	}

	mySpriteInstances[aSprite->GetRenderOrder()].emplace_back(aSprite);

	return true;
}
//PhysX
bool CScene::AddPXScene(PxScene* aPXScene)
{
	if (!aPXScene) {
		return false;
	}
	myPXScene = aPXScene;
	return true;
}
//POPULATE SCENE END
//REMOVE SPECIFIC INSTANCE START
bool CScene::RemoveInstance(CPointLight* aPointLight)
{
	for (int i = 0; i < myPointLights.size(); ++i)
	{
		if (aPointLight == myPointLights[i])
		{
			//std::swap(myGameObjects[i], myGameObjects[myGameObjects.size() - 1]);
			//myGameObjects.pop_back();
			myPointLights.erase(myPointLights.begin() + i);
		}
	}
	return true;
}

bool CScene::RemoveInstance(CAnimatedUIElement* anAnimatedUIElement)
{
	for (int i = 0; i < myAnimatedUIElements.size(); ++i)
	{
		if (myAnimatedUIElements[i] == anAnimatedUIElement)
		{
			myAnimatedUIElements.erase(myAnimatedUIElements.begin() + i);
		}
	}
	return true;
}

bool CScene::RemoveInstance(CGameObject* aGameObject)
{
	for (int i = 0; i < myGameObjects.size(); ++i)
	{
		if (aGameObject == myGameObjects[i])
		{
			//std::swap(myGameObjects[i], myGameObjects[myGameObjects.size() - 1]);
			//myGameObjects.pop_back();
			myGameObjects.erase(myGameObjects.begin() + i);
		}
	}
	return true;
}
//REMOVE SPECIFIC INSTANCE END
//CLEAR SCENE OF INSTANCES START
bool CScene::ClearPointLights()
{
	for (auto& p : myPointLights)
	{
		delete p;
		p = nullptr;
	}
	myPointLights.clear();
	return true;
}

bool CScene::ClearLineInstances()
{
	for (size_t i = 0; i < myLineInstances.size(); ++i)
	{
		if (myLineInstances[i] != nullptr)
		{
			delete myLineInstances[i];
			myLineInstances[i] = nullptr;
		}
	}
	return false;
}

bool CScene::ClearVFXInstances()
{
	for (auto& vfx : myVFXInstances)
	{
		delete vfx;
		vfx = nullptr;
	}
	myVFXInstances.clear();
	return false;
}

bool CScene::ClearAnimatedUIElement()
{
	for (size_t i = 0; i < myAnimatedUIElements.size(); ++i)
	{
		delete myAnimatedUIElements[i];
		myAnimatedUIElements[i] = nullptr;
	}
	myAnimatedUIElements.clear();
	return false;
}

bool CScene::ClearTextInstances()
{
	for (auto& text : myTexts)
	{
		delete text;
		text = nullptr;
	}
	myTexts.clear();

	return false;
}

bool CScene::ClearGameObjects()
{
	for (auto& gameObject : myGameObjects)
	{
		delete gameObject;
		gameObject = nullptr;
	}
	myGameObjects.clear();
	return true;
}

bool CScene::ClearSprites()
{

	for (UINT i = 0; i < mySpriteInstances.size() - 1; ++i)
	{
		for (auto& sprite : mySpriteInstances[static_cast<ERenderOrder>(i)])
		{
			delete sprite;
			sprite = nullptr;
		}
	}
	mySpriteInstances.clear();

	return true;
}
//CLEAR SCENE OF INSTANCES START

bool CScene::NearestPlayerComparer::operator()(const CPointLight* a, const CPointLight* b) const
{
	float dist0 = Vector3::DistanceSquared(a->GetPosition(), myPos);
	float dist1 = Vector3::DistanceSquared(b->GetPosition(), myPos);
	return dist0 < dist1;
}


//FUNCTIONS THAT NEED TO BE REFACTORED OR REMOVED
//CCameraComponent* CScene::MainCamera()
//{
//	return myMainCamera;
//}
//
//CEnvironmentLight* CScene::EnvironmentLight()
//{
//	return myEnvironmentLight;
//}
//
//SNavMesh* CScene::NavMesh()
//{
//	return myNavMesh;
//}
//void CScene::UpdateLightsNearestPlayer()
//{
//	if (myPlayer == nullptr) {
//		return;
//	}
//
//	ourNearestPlayerComparer.myPos = myPlayer->myTransform->Position();
//	std::sort(myPointLights.begin(), myPointLights.end(), ourNearestPlayerComparer);
//}

//REMOVE? THIS FUNCTIONALITY HAS BEEN COMMENTED SINCE BEFORE THE SCENE REFACTOR OF 2021-01-26
//std::pair<unsigned int, std::array<CPointLight*, LIGHTCOUNT>> CScene::CullLights(const DirectX::SimpleMath::Vector3& aPosition)
//{
//	std::pair<unsigned int, std::array<CPointLight*, LIGHTCOUNT>> pointLightPair;
//	UINT counter = 0;
//	for (UINT i = 0; i < myPointLights.size(); ++i)
//	{
//		float distanceSquared = DirectX::SimpleMath::Vector3::DistanceSquared(myPointLights[i]->GetPosition(), aPosition);
//		float range = myPointLights[i]->GetRange();
//
//		if (distanceSquared < (range * range))
//		{
//			pointLightPair.second[counter] = myPointLights[i];
//			++counter;
//
//			if (counter == 8)
//			{
//				break;
//			}
//		}
//	}
//	pointLightPair.first = counter;
//	return pointLightPair;
//}
//
//std::pair<unsigned int, std::array<CPointLight*, LIGHTCOUNT>> CScene::CullLights(const std::vector<DirectX::SimpleMath::Matrix>& somePositions) const
//{
//
//
//	std::pair<unsigned int, std::array<CPointLight*, LIGHTCOUNT>> pointLightPair;
//
//	unsigned int counter = 0;
//	for (unsigned int pointLightIndex = 0; pointLightIndex < myPointLights.size(); ++pointLightIndex)
//	{
//		for (unsigned int positionIndex = 0; positionIndex < somePositions.size(); ++positionIndex)
//		{
//			float distanceSquared = DirectX::SimpleMath::Vector3::DistanceSquared(myPointLights[pointLightIndex]->GetPosition(), somePositions[positionIndex].Translation());
//			float range = myPointLights[pointLightIndex]->GetRange();
//
//			if (distanceSquared < (range * range))
//			{
//				pointLightPair.second[counter] = myPointLights[pointLightIndex];
//				++counter;
//				break;
//			}
//		}
//
//		if (counter >= 8)
//		{
//			break;
//		}
//	}
//	pointLightPair.first = counter;
//	return pointLightPair;
//}
//bool CScene::AddEnemies(CGameObject* aEnemy)
//{
//	if (!aEnemy)
//	{
//		return false;
//	}
//	myEnemies.emplace_back(aEnemy);
//	return true;
//}
//
//bool CScene::AddBoss(CGameObject* aBoss)
//{
//	if (!aBoss)
//	{
//		return false;
//	}
//	myBoss = aBoss;
//	return true;
//}

//bool CScene::AddDestructible(CGameObject* aDestructible)
//{
//	if (!aDestructible)
//	{
//		return false;
//	}
//	myDestructibles.emplace_back(aDestructible);
//	return true;
//}

//bool CScene::AddPlayer(CGameObject* aPlayer)
//{
//	if (!aPlayer)
//	{
//		return false;
//	}
//	myPlayer = aPlayer;
//	return true;
//}
//void CScene::SetPlayerToOutline(CGameObject* aPlayer)
//{
//	//auto it = std::find(myGameObjects.begin(), myGameObjects.end(), aPlayer);
//	//if (it != myGameObjects.end())
//	//{
//	//	std::swap(*it, myGameObjects.back());
//	//	myGameObjects.pop_back();
//	//}
//	/*if (myModelToOutline) {
//		myGameObjects.emplace_back(std::move(myModelToOutline));
//	}
//	auto it = std::find(myGameObjects.begin(), myGameObjects.end(), aGameObject);
//	if (it != myGameObjects.end()) {
//		std::swap(*it, myGameObjects.back());
//		myModelToOutline = myGameObjects.back();
//		myGameObjects.pop_back();
//	}
//	else {*/
//	myModelsToOutline[0] = aPlayer;
//	//}
//}

//void CScene::SetEnemyToOutline(CGameObject* anEnemy)
//{
//	myModelsToOutline[1] = anEnemy;
//}
//void CScene::TakeOwnershipOfAIBehavior(IAIBehavior* aBehavior)
//{
//	myEnemyBehavior = aBehavior;
//}