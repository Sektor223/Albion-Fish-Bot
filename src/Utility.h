#pragma once

#include<Windows.h>
#include<string>
#include<atomic>

enum Status
{
	STOPPED,
	STARTED,
	LOOKING,
	FOUND,
	CATCH,
	PULL,
	RELEASE,
	FINISHED
};

struct AppState {
private:
	//cv::Mat getMat();

public:
	//numbers
	float f = 0.0f;
	float mainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0,0 }, MONITOR_DEFAULTTOPRIMARY));
	int counter = 0;
	int areaRadius = 200;
	int mainWinSize = (int)(500 * mainScale);
	int hostWinSize = (int)(1 * mainScale);


	//keys will be in other struct probably
	int fihKey = VK_NUMPAD5;
	int stopFih = VK_ESCAPE;

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
	
	std::string statusMessage = "never started";

	ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
	//cv::Mat fullScale; 

	std::atomic<cv::Rect> boundRect = cv::Rect();

};

//вынести в отдельный файл
struct Vision
{
private:
	AppState& state;

	static inline HWND windowDesk = nullptr;
	static inline cv::Mat img = cv::Mat();
	static inline cv::Mat fullScale = cv::Mat();
	static inline bool init = false;
	static inline bool areaSelected = false;

	cv::Mat	imgHSV, imgMask;
	std::vector<std::vector<cv::Point>> contours;
	::RECT selectedArea = { 0 };
	
	const std::vector<std::vector<int>> objHSV = {
		//hmin, hmax, smin, smax, vmin, vmax
			{0, 14, 80, 207, 124, 255} //BOBBER

	};

	const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	enum objType
	{
		BOBBER = 0
	};
	enum HSV
	{
		HMIN = 0, HMAX = 1,
		SMIN = 2, SMAX = 3,
		VMIN = 4, VMAX = 5
	};

public: 
	const std::string winName = "Debug Window";

	Vision(AppState& appState) : state(appState) {
		if (!init) {
			initWindow();
		}
	}
	
	void startCapture() {
		if (!areaSelected) {
			selectAreaWithMouse();
		}
		getDesktopMat();
		getImage();
		showImage();
	}

	void stopCapture()
	{
		cv::destroyWindow(winName);
		areaSelected = false;
	}

	//в функциях могли остаться ненужные куски state, которые можно перенести в класс

private:

	bool initWindow() {
		windowDesk = GetDesktopWindow();
		init = true;
		return init;
	}
	//todo: fix constuctor
	cv::Mat cropMat(const cv::Mat& fullFrame, const RECT& area) {
		int x = max(0, area.left);
		int y = max(0, area.top);
		int width = min(fullFrame.cols - x, area.right - area.left);
		int height = min(fullFrame.rows - y, area.bottom - area.top);

		if (width <= 0 || height <= 0) {
			return cv::Mat(); // Пустая матрица при невалидной области
		}

		return fullFrame(cv::Rect(x, y, width, height)).clone();
	}

	void getMaskColorBased(cv::Mat& imgHsv, cv::Mat& imgMask)
	{
		cv::Scalar Lower(objHSV[BOBBER][HMIN],
			             objHSV[BOBBER][SMIN],
			             objHSV[BOBBER][VMIN]);

		cv::Scalar Upper(objHSV[BOBBER][HMAX],
			             objHSV[BOBBER][SMAX],
			             objHSV[BOBBER][VMAX]);

		inRange(imgHsv, Lower, Upper, imgMask);

	}

	void showImage()
	{
		cv::namedWindow(winName);
		cv::imshow(winName, img);
		cv::waitKey(5);
	};

	void selectAreaWithMouse() {

		POINT cursorPos;

		// ожидаем нажатие Num5 
		while (true) {

			if (GetAsyncKeyState(state.fihKey) & 0x8000) {

				while (GetAsyncKeyState(state.fihKey) & 0x8000) {
					Sleep(10);
				}


				GetCursorPos(&cursorPos);
				ScreenToClient(windowDesk, &cursorPos);

				// Вычисляем область вокруг курсора
				selectedArea.left = cursorPos.x - state.areaRadius / 2;
				selectedArea.top = cursorPos.y - state.areaRadius / 2;
				selectedArea.right = cursorPos.x + state.areaRadius / 2;
				selectedArea.bottom = cursorPos.y + state.areaRadius / 2;
				break;
			}
			Sleep(10);
		}
		areaSelected = true;
	};

	//need to optimize?
	void getDesktopMat() {
		HDC deviceContext = GetDC(windowDesk);
		HDC memoryDeviceContext = CreateCompatibleDC(deviceContext);

		HBITMAP bitmap = CreateCompatibleBitmap(deviceContext, screenWidth, screenHeight);

		SelectObject(memoryDeviceContext, bitmap);

		//copy data into the bitmap
		BitBlt(memoryDeviceContext, 0, 0, screenWidth, screenHeight, deviceContext, 0, 0, SRCCOPY);

		//specify format by using bitmap info header
		BITMAPINFOHEADER bi;
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = screenWidth;
		bi.biHeight = -screenHeight;
		bi.biPlanes = 1;
		bi.biBitCount = 32;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0; //cause no compression
		bi.biXPelsPerMeter = 1;
		bi.biYPelsPerMeter = 2;
		bi.biClrUsed = 3;
		bi.biClrImportant = 4;

		fullScale = cv::Mat(screenHeight, screenWidth, CV_8UC4);//rgba = 8bit per value

		//copy and transform data
		GetDIBits(memoryDeviceContext, bitmap, 0, screenHeight, fullScale.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

		DeleteObject(bitmap);
		DeleteDC(memoryDeviceContext);
		ReleaseDC(windowDesk, deviceContext);

	};

	void getImage()
	{
		img = cropMat(fullScale, selectedArea);

		cvtColor(img, imgHSV, cv::COLOR_BGR2HSV);

		getMaskColorBased(imgHSV, imgMask);

		//find countour
		findContours(imgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		//drawContours(bobberBack, contours, -1, (0, 255, 0), 3);


		//find rectangle for bobber
		for (size_t i = 0; i < contours.size(); ++i) {
			state.boundRect = cv::boundingRect(contours[i]);
			cv::Rect boundRect = state.boundRect.load();
			if (boundRect.area() > 400)
			{                                                                //&& (state.boundRect.width < 70 || state.boundRect.height < 70)
				cv::rectangle(img, boundRect.tl(), boundRect.br(), cv::Scalar(0, 0, 255), 3);
			}

		}
	};
	
};