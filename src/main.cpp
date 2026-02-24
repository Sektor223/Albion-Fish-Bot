#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

#include"tgbot/tgbot.h"
//#include<Windows.h>
#include<dwmapi.h>
#include<d3d11.h>
#include<tchar.h>
#include<thread>
#include<chrono>

#include<imgui/imgui.h>
#include<imgui/imgui_impl_dx11.h>
#include<imgui/imgui_impl_win32.h>

TgBot::Bot bot("8453817061:AAFzZ0Xl6C8VivHLaw_V6bcb7Io1Uf0Mw6k");
//#include <wininet.h>

#include"Config.h"
#include"Notifier.h"
#include"Vision.h"
#include"Utility.h"
#include"Login.h"

//#include <windows.h>

//debug
#include<iostream>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
//debug
//#include <shellscalingapi.h>
//#pragma comment(lib, "Shcore.lib")
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void CreateConsole()
{
	AllocConsole();

	// Перенаправление стандартного вывода (std::cout)
	HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hConHandle = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
	FILE* fp = _fdopen(hConHandle, "w");
	freopen_s(&fp, "CONOUT$", "w", stdout);

	// Перенаправление стандартного ввода (std::cin)
	stdHandle = GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "r");
	freopen_s(&fp, "CONIN$", "r", stdin);

	// Перенаправление стандартного ошибок (std::cerr)
	stdHandle = GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	freopen_s(&fp, "CONOUT$", "w", stderr);

	// Очищаем буферы, чтобы все сразу попадало в консоль
	std::ios::sync_with_stdio();
}



ImGuiStyle SetupImGuiStyle();

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//TgBot::Bot bot("8453817061:AAFzZ0Xl6C8VivHLaw_V6bcb7Io1Uf0Mw6k");
//std::thread run;
// do NOT remove checkAuthenticated(), it MUST stay for security reasons
//std::thread check; // do NOT remove this function either.


INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {
	
	
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Make process DPI aware and obtain main monitor scale
	ImGui_ImplWin32_EnableDpiAwareness();
	float mainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0,0 }, MONITOR_DEFAULTTOPRIMARY));
	

	//CreateConsole();
	//std::cout << "test" << std::endl;
	
	
	config.loadConfig();
	//Notifier::brokenMessage(bot, config.idBuff);
	//window
	//HICON hIcon = LoadIconWithScaleDown(NULL, L"src/fih.png", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	HICON hIcon = (HICON)LoadImage(NULL, L"src/fih.ico", IMAGE_ICON, 256, 256, LR_LOADFROMFILE);
	WNDCLASSEXW wc = { sizeof(wc), ACS_TRANSPARENT, WndProc, 0L, 0L, GetModuleHandle(nullptr), hIcon, nullptr, nullptr, nullptr, L"JustFih", hIcon };
	::RegisterClassExW(&wc);
	HWND hwnd = ::CreateWindowExW(WS_EX_TOPMOST |  WS_EX_LAYERED, wc.lpszClassName, L"ImJustAfih", WS_POPUP, 0, 0, screenWidth, screenHeight, nullptr, nullptr, wc.hInstance, nullptr);
	

	//SetLayeredWindowAttributes(hwnd, 0, RGB(0, 0, 0), LWA_ALPHA);
	SetLayeredWindowAttributes(hwnd, 0, RGB(0, 0, 0), LWA_COLORKEY);
	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(hwnd, &margins);

	//init d3d
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);
	//setup imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImFont *mainFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Arial.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
	io.FontDefault = mainFont;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	
	//style and scaling
	ImGuiStyle style;
	ImGui::StyleColorsDark();
	//ImGuiStyle style = SetupImGuiStyle();
	style.ScaleAllSizes(mainScale);
	style.FontScaleDpi = mainScale;
	io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
	io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.


	 //When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);


	vizu.init_g_pd3dDevice_(g_pd3dDevice);

	//keyAuthInit();
	isLogged = true;//keySucces();
	//KeyAuthApp.check(); // do NOT specify true usually, it is slower and will get you blocked from API
	//if (!KeyAuthApp.response.success) {
	//	MessageBoxA(NULL, KeyAuthApp.response.message.c_str(), "Error", MB_OK | MB_ICONERROR);
	//	Sleep(2500);
	//	exit(0);
	//}

	
	if (isLogged.load()) {
		futureLogin loginG;
		state.setMainTitle(loginG.getTitle());
	}
	


	//main loop
	while (!state.shouldExit.load())
	{
		
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				state.shouldExit = true;

		}
		if (state.shouldExit.load())
			break;


		//handle window mini,azed or screen locked
		if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
		{
			::Sleep(10);
			continue;
		}
		g_SwapChainOccluded = false;

		//handle resize
		if (g_ResizeHeight != 0 && g_ResizeWidth != 0)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			g_ResizeWidth = g_ResizeHeight = 0;
			CreateRenderTarget();
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		

		if (isLogged.load()) {
			
		    state.manage();
			//if (!run.joinable()) {
			//	run = std::thread(checkAuthenticated, ownerid);
			//	// do NOT remove checkAuthenticated(), it MUST stay for security reasons

			//}
			//if (!check.joinable()) {
			//	check = std::thread(sessionStatus); // do NOT remove this function either.
			//}
			

		   if (state.fihing.load()) {

			  if (!state.fishingThread.joinable()) {
				state.fishingThread = std::thread(&Vision::mainCapture, &vizu, std::ref(state.fihing), std::ref(state.shouldExit));
			  }

			  if (GetAsyncKeyState(config.stopFihKey) & 0x8000) {
				state.fihing = false;
			  }

			  if (vizu.getSelectAreaState() && vizu.getTimerStatus()) {
				auto clockEnd = std::chrono::high_resolution_clock::now();
				vizu.duration = std::chrono::duration_cast<std::chrono::seconds>(clockEnd - vizu.clockStart).count();
				if (vizu.duration >= 360) {
					state.fihing = false;
					Notifier::brokenMessage(bot ,config.idBuff);
				}
			  }

		   }
		   else {
			  if (state.fishingThread.joinable()) {
				state.fishingThread.join();
				vizu.duration = 0;
			  }

		   }

	    }
		else {
			//AuthorizationWindow();
			
		}
		
			
			//rendering
			ImGui::Render();
			const float clear_color_with_alpha[4] = { 0, 0, 0, 0 };
			g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
			g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			//SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

			// Update and Render additional Platform Windows
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			
			//present
			HRESULT hr = g_pSwapChain->Present(1, 0);
			g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
		}
		isLogged = false;
	config.saveConfig();
	if (state.fishingThread.joinable()) {
		state.fishingThread.join();
	}
	/*if (run.joinable()) {
		run.detach();
	}
	if (check.joinable()) {
		check.detach();
	}*/
	if (state.tgBotThread.joinable()) {
		state.tgBotThread.detach();
	}
	

		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		CleanupDeviceD3D();
		::DestroyWindow(hwnd);
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);

		return 0;
	};



	



bool CreateDeviceD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice ,&featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED)
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain)        { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice)        { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView)
	{
		g_mainRenderTargetView->Release();
		g_mainRenderTargetView = nullptr;
	}
}





// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	switch (msg)
	{
	/*case WM_KEYDOWN:
		if (state.confOpen) {
			if (state.fihClicked) {
				GetKeyNameTextA(lParam, cnf.fihKeyname, sizeof(cnf.fihKeyname));
			}
			if (state.stopfihClicked) {
				GetKeyNameTextA(lParam, cnf.stopFihKeyname, sizeof(cnf.stopFihKeyname));
			}
		}
		
		return EXIT_SUCCESS;*/

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) //Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

ImGuiStyle SetupImGuiStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.5f;
	style.WindowPadding = ImVec2(11.7f, 6.0f);
	style.WindowRounding = 3.3f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(20.0f, 9.9f);
	style.FrameRounding = 0.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 14.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.58431375f, 0.59607846f, 0.6156863f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0627451f, 0.06666667f, 0.08627451f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.10980392f, 0.11372549f, 0.13333334f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.10980392f, 0.11372549f, 0.13333334f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0627451f, 0.06666667f, 0.08627451f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.05882353f, 0.5294118f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.05882353f, 0.5294118f, 0.9764706f, 0.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.047058824f, 0.050980393f, 0.0627451f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.10980392f, 0.11372549f, 0.13333334f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.14509805f, 0.14901961f, 0.18431373f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4862745f, 0.4862745f, 0.4862745f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 0.22746783f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.81960785f, 0.81960785f, 0.81960785f, 0.3304721f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.22745098f, 0.44313726f, 0.75686276f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.20784314f, 0.47058824f, 0.8509804f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.05882353f, 0.5294118f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.3882353f, 0.3882353f, 0.3882353f, 0.62f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.13725491f, 0.4392157f, 0.8f, 0.78f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.13725491f, 0.4392157f, 0.8f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.34901962f, 0.34901962f, 0.34901962f, 0.17f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.95f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.0f, 0.4745098f, 1.0f, 0.931f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.20784314f, 0.20784314f, 0.20784314f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.91764706f, 0.9254902f, 0.93333334f, 0.9862f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.7411765f, 0.81960785f, 0.9137255f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.3882353f, 0.3882353f, 0.3882353f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.44705883f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.7764706f, 0.8666667f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.5686275f, 0.5686275f, 0.6392157f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.6784314f, 0.6784314f, 0.7372549f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.29803923f, 0.29803923f, 0.29803923f, 0.09f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.95f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.69803923f, 0.69803923f, 0.69803923f, 0.7f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.2f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.35f);

	return style;
}
