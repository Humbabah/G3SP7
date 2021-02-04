#pragma once
#include <vector>
#include <string>

class CGraphManager;
struct ImFontAtlas;

class CImguiManager
{
public:
	CImguiManager();
	~CImguiManager();
	void Render();
	void PostRender();
	void DebugWindow();
	void LevelSelect();
	void LevelsToSelectFrom(std::vector<std::string> someLevelsToSelectFrom);
private:
	CGraphManager* myGraphManager;
	const std::string GetSystemMemory();
	const std::string GetDrawCalls();
	std::vector<std::string> myLevelsToSelectFrom;
	bool myGraphManagerIsFullscreen;
};
