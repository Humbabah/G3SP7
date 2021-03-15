#pragma once
#include "Window.h"

namespace IronWroughtImGui {

	//struct VFXSubWindow {
	//	bool myIsOpen;
	//	std::string myName;
	//};

	class CCurveWindow : public CWindow
	{
	public:
		CCurveWindow(const char* aName);
		~CCurveWindow() override;

		void OnEnable()	override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:

		std::unordered_map<std::string, float[4]> myValueMap;

		//float myValues[4];

	};
}

