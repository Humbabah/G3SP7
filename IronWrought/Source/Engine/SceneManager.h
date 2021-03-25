#pragma once
#include "Scene.h"
#include "JsonReader.h"

class CSceneManager
{
public:
	CSceneManager();
	~CSceneManager();
	static CScene* CreateEmpty();
	static CScene* CreateScene(const std::string& aSceneName);

private:
	static bool AddGameObjects(CScene& aScene, RapidArray someData);
	static void SetTransforms(CScene& aScene, RapidArray someData);
	static void AddModelComponents(CScene& aScene, RapidArray someData);
	static void SetVertexPaintedColors(CScene& aScene, RapidArray someData);

	static void AddInstancedModelComponents(CScene& aScene, RapidArray someData);
	static void AddPointLights(CScene& aScene, RapidArray someData);
	static void AddDecalComponents(CScene& aScene, RapidArray someData);
	static void AddPlayer(CScene& aScene);
};


#include <future>
class CSceneFactory {
	friend class CEngine;
public:
	static CSceneFactory* Get();
	void LoadSceneAsync(const std::string& aSceneName, std::function<void()> onComplete);
	void Update();

private:
	CSceneFactory();
	~CSceneFactory();
	static CSceneFactory* ourInstance;

private:
	std::future<CScene*> myFuture;
	std::function<void()> myOnComplete;
};


