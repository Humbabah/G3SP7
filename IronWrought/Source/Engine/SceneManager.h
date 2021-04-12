#pragma once
#include "Scene.h"
#include "JsonReader.h"
#include "StateStack.h"

#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <concurrent_vector.h>

#include <future>


struct SVertexPaintCollection;

enum class ColliderType {
	none = 0,
	BoxCollider = 1,
	SphereCollider = 2,
	CapsuleCollider = 3,
	MeshCollider = 4
};

//struct SSceneSetup {
//	concurrency::concurrent_vector<CGameObject*> myGameObjects;
//};

class CSceneManager
{
public:
	CSceneManager();
	~CSceneManager();
	static CScene* CreateEmpty();
	static CScene* CreateScene(const std::string& aSceneName);
	static CScene* CreateMenuScene(const std::string& aSceneName, const std::string& aCanvasPath);

private:
	static CScene* Instantiate();

	static bool AddGameObjects(CScene& aScene, RapidArray someData);
	static void SetTransforms(CScene& aScene, RapidArray someData);
	static void AddModelComponents(CScene& aScene, RapidArray someData);
	static void SetVertexPaintedColors(CScene& aScene, RapidArray someData, const SVertexPaintCollection& vertexColorData);

	static void AddInstancedModelComponents(CScene& aScene, RapidArray someData);
	static void AddDirectionalLight(CScene& aScene, RapidObject someData);
	static void AddPointLights(CScene& aScene, RapidArray someData);
	static void AddDecalComponents(CScene& aScene, RapidArray someData);
	static void AddCollider(CScene& aScene, RapidArray someData);
	static void AddPlayer(CScene& aScene);
	static void AddEnemyComponents(CScene& aScene, RapidArray someData);
	static CScene* ourLastInstantiatedScene;
};

struct CSceneSetup {
	concurrency::concurrent_vector<CGameObject*> myGameObjects;
};

class CSceneFactory {
	friend class CEngine;
public:
	static CSceneFactory* Get();
	void LoadScene(const std::string& aSceneName, const CStateStack::EState aState, std::function<void(std::string)> onComplete = nullptr);
	void LoadSceneAsync(const std::string& aSceneName, const CStateStack::EState aState, std::function<void(std::string)> onComplete);
	void Transition(const std::string& aFromScene, const std::string& aToScene, const std::string& aTransitionScene);
	void Update();

private:
	void OnTransitionComplete();
	CSceneSetup FillSceneAsync(const std::string& aSceneName, std::function<void()> onComplete);

private:
	CSceneFactory();
	~CSceneFactory();
	static CSceneFactory* ourInstance;

private:

	std::future<CSceneSetup> myTransitionThread;

	std::future<CScene*> myFuture;
	std::function<void(std::string)> myOnComplete;
	std::string myLastSceneName;
	CStateStack::EState myLastLoadedState;
};
