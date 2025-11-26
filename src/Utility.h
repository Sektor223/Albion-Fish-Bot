#pragma once
#include <imgui/imgui_impl_win32.h>
#include <WinUser.h>

namespace binds 
{
	inline int fihKey = VK_NUMPAD5;
	inline int stopFih = VK_ESCAPE;
}

struct AppState {
public:
	//numbers
	float f = 0.0f;
	float mainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0,0 }, MONITOR_DEFAULTTOPRIMARY));
	int counter = 0;
	int areaRadius = 200;
	int mainWinSize = (int)(500 * mainScale);
	int hostWinSize = (int)(1 * mainScale);


	

	//bool
	bool showDemoWindow = false;
	bool showAnotherWindow = false;
	bool showMainWindow = true;
	bool showHint = true;
	bool exit = false;
	std::atomic<bool> fihing = false;
	bool debug = false;

	//window flags
	ImGuiWindowFlags flazhoks = 
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;

	ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
	

};


