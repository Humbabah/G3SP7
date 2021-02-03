#include "stdafx.h"
#include "ImguiManager.h"
#include <imgui.h>
#include "Engine.h"
#include "Scene.h"
#include "SceneManager.h"
#pragma comment(lib, "imgui.lib")

#include "RenderManager.h"
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

static ImFont* ImGui_LoadFont(ImFontAtlas& atlas, const char* name, float size, const ImVec2& displayOffset = ImVec2(0, 0))
{
	char* windir = nullptr;
	if (_dupenv_s(&windir, nullptr, "WINDIR") || windir == nullptr)
		return nullptr;

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0104, 0x017C, // Polish characters and more
		0,
	};

	ImFontConfig config;
	config.OversampleH = 4;
	config.OversampleV = 4;
	config.PixelSnapH = false;

	auto path = std::string(windir) + "\\Fonts\\" + name;
	auto font = atlas.AddFontFromFileTTF(path.c_str(), size, &config, ranges);
	if (font)
		font->DisplayOffset = displayOffset;

	free(windir);

	return font;
}
ImFontAtlas myFontAtlas;

CImguiManager::CImguiManager()
{
	ImGui::DebugCheckVersionAndDataLayout("1.80 WIP", sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(unsigned int));
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_LoadFont(myFontAtlas, "segoeui.ttf", 22.0f);
	myFontAtlas.Build();

	ImGui::CreateContext(&myFontAtlas);
}

CImguiManager::~CImguiManager()
{
	ImGui::DestroyContext();
}

void CImguiManager::DebugWindow()
{
	if (ImGui::Begin("Debug info"))
	{
		ImGui::Text("Framerate: %.0f", ImGui::GetIO().Framerate);
		ImGui::Text(GetSystemMemory().c_str());
		ImGui::Text(GetDrawCalls().c_str());
	}
	ImGui::End();
}

void CImguiManager::LevelSelect()
{
	//std::vector<std::string> files = CJsonReader::GetFilePathsInFolder(ASSETPATH + "Assets/Generated");
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 200, 0));
	float x = 10.f + ((ImGui::GetFontSize() + 5.5f) * static_cast<float>(myLevelsToSelectFrom.size()));
	ImGui::SetNextWindowSize({200.f,  x});

	ImGui::Begin("LevelSelect", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	/*if (ImGui::TreeNodeEx("Selection State: Single Selection", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf))
	{*/
		static int selected = -1;
		for (int n = 0; n < myLevelsToSelectFrom.size(); n++)
		{
			char buf[512];
			sprintf_s(buf, "%s", myLevelsToSelectFrom[n].c_str());

			if (ImGui::Selectable(buf, selected == n, ImGuiSelectableFlags_AllowDoubleClick))
			{
				selected = n;

				if (ImGui::IsMouseDoubleClicked(0))
				{
					std::cout << "Load Level: " << buf << std::endl;
					CScene* myUnityScene = CSceneManager::CreateScene(buf);
					CEngine::GetInstance()->AddScene(CStateStack::EState::InGame, myUnityScene);
					CEngine::GetInstance()->SetActiveScene(CStateStack::EState::InGame);
				}

			}
		}
	/*	ImGui::TreePop();
	}*/


	ImGui::End();
}

const std::string CImguiManager::GetSystemMemory()
{
	// From TGA2D
	PROCESS_MEMORY_COUNTERS memCounter;
	BOOL result = GetProcessMemoryInfo(GetCurrentProcess(),
		&memCounter,
		sizeof(memCounter));

	if (!result)
	{
		return "";
	}

	SIZE_T memUsed = (memCounter.WorkingSetSize) / 1024;
	SIZE_T memUsedMb = (memCounter.WorkingSetSize) / 1024 / 1024;

	std::string mem = "System Memory: ";
	mem.append(std::to_string(memUsed));
	mem.append("Kb (");
	mem.append(std::to_string(memUsedMb));
	mem.append("Mb)");

	return mem;
}

const std::string CImguiManager::GetDrawCalls()
{
	std::string drawCalls = "Draw Calls: ";
	drawCalls.append(std::to_string(CRenderManager::myNumberOfDrawCallsThisFrame));
	return drawCalls;
}

void CImguiManager::LevelsToSelectFrom(std::vector<std::string> someLevelsToSelectFrom)
{
	myLevelsToSelectFrom = someLevelsToSelectFrom;
}