#pragma once

class CGraphManager;
struct SPin;
class CJsonReader;

class CSaveLoadGraphManager
{
public:
	void LoadScripts(CGraphManager& aGraphManager, const std::string& aSceneName, std::string& aSceneFolder);
	void LoadTreeFromFile(CGraphManager& aGraphManager);
#ifdef _DEBUG
	void SaveTreeToFile(CGraphManager& aGraphManager);

	void SaveNodesToClipboard(CGraphManager& aGraphManager);
	void LoadNodesFromClipboard(CGraphManager& aGraphManager);
#endif // _DEBUG

private:
};

