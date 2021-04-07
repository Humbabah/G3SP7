#include "stdafx.h"
#include "Game.h"
#include "InGameState.h"

//#include "MainSingleton.h"
//#include "PostMaster.h"

#ifdef _DEBUG
#pragma comment(lib, "Engine_Debug.lib")
#endif // _DEBUG
#ifdef NDEBUG
#pragma comment(lib, "Engine_Release.lib")
#endif // NDEBUG

CGame::CGame()
{
}

CGame::~CGame()
{
	/*std::cout << __FUNCTION__ << std::endl;*/ // Aki 2021 04 06 to test something, ok to remove
}

void CGame::Init()
{
#ifdef NDEBUG
	InitRealGame();
#else
	InitDev();
#endif

}

bool CGame::Update()
{
	bool stateStackHasUpdated = myStateStack.Update();
	CMainSingleton::PostMaster().FlushEvents();
	return stateStackHasUpdated;
}

void CGame::InitDev()
{
	myStateStack.Awake(
		{
			//CStateStack::EState::LoadLevel,
			CStateStack::EState::InGame,
			CStateStack::EState::PauseMenu,
			CStateStack::EState::MainMenu
		},
		CStateStack::EState::InGame);
}

void CGame::InitRealGame()
{
	myStateStack.Awake(
		{
			CStateStack::EState::BootUp,
			CStateStack::EState::MainMenu,
			CStateStack::EState::InGame,
			CStateStack::EState::PauseMenu
		},
		CStateStack::EState::BootUp);
}
