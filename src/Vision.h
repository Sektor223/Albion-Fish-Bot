#pragma once

#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
//#include <Windows.h>
#include<keyauth/skStr.h>
#include <wrl/client.h>
#include <d3d11.h>

#include"Config.h"

//#include <shellscalingapi.h>
//#pragma comment(lib, "Shcore.lib")

//SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

 //mb need to create globalVar header
inline int screenWidth = GetSystemMetrics(SM_CXSCREEN);
inline int screenHeight = GetSystemMetrics(SM_CYSCREEN);


class Vision
{
private:
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
	enum Status
	{
		STARTED,
		LOOKING,
		FOUND,
		CATCH,
		FINISHED,
		SERVER_RESTART
	};

	//for debug window display
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> textureSRV = nullptr;
	static inline ImTextureID imguiTexture = NULL;
	std::string statusMessage = skCrypt("never started").decrypt();

	
	//for getDesktopMat
	HDC deviceContext = nullptr;
	HDC memoryDeviceContext = nullptr;
	HBITMAP bitmap = nullptr;
	BITMAPINFOHEADER bi;
	bool gdiInitialized = false;
	
	int windowWidth;
	int windowHeight;
	
	int clientRectLeft;
	int clientRectTop;
	//used in getDesktopMat
	cv::Mat fullScale = cv::Mat();
	/////////////////////////
	cv::Mat halved;
	/// <summary>
	/// center for both windowed capture and fullscreen capture
	/// </summary>
	POINT windowCenter;
	//in vision class constructor
	Config& config;
	//in vision class constructor
	ID3D11Device* g_pd3dDevice_;
	///////////////////////////
	
	
	//must be reset when status == FINISHED
	static inline INPUT inputCatch = { 0 };
	
	//used in catchProcess
	int scalePosDOWN = 135;
	//used in catchProcess
	int scalePosUP = 150;

	//used in initWindow
	bool init = false;
	//used in initWindow
	HWND targetHWND = nullptr;
	//HWND targetWindow = nullptr;
	//used in selectAreaWithMouse
	::RECT selectedArea = { 0 };
	bool isAreaSelected = false;

	//for overal logic control
	Status status = STARTED;
	int cleanCounter = 0;
	int emptyCounter = 0;
	POINT itThatRemember;
	
	//reset when fishing cycle end
	cv::Rect cropRect = cv::Rect();
	//reset when fishing cycle end
	cv::Rect scaleRect = cv::Rect();
	POINT matchingPos = POINT();
	POINT clientPos;
	cv::Mat displayImage;
	//used in getImage
	cv::Mat	img, imgHSV, imgMask;
	std::vector<std::vector<cv::Point>> contours;
	//used in getImage and area comparsion for logic control
	cv::Rect boundRect = cv::Rect();
	//это для 1920x1080
	//c высоты  800-875   ---- клюнуло  209-450
	//с средней 1500-1700 ---- клюнуло  252-644
	//c низов   2400-2900 ---- клюнуло  546-975
	//актуальное записано в симпленоте с гаус блюром
	int inWaterSizeMin;
	
	bool buffsActive = false;
	std::chrono::steady_clock::time_point timeFoodStart;
	cv::Rect foundRect;
	int baitCounter = 0;
	bool timerActive = true;

	int restCounter = 0;

	//add new objects here
	const std::vector<std::vector<int>> objHSV = {
		//hmin, hmax, smin, smax, vmin, vmax
			{0, 14, 80, 207, 124, 255} //BOBBER

	};
	
	//template for matching/////////////////////////////////////////////////////////
	enum matchingEnum {
		SCALE,
		PIE, //dont use directly, use SLOT instead
		SALAD,//dont use directly, use SLOT instead
		BAIT,
		LOGS, //clean inventory
		SLOT,
		MAINLOGO, //restart check
		LOGINBUTTON, //restart check
		ENTERWORLDBUTTON, //restart check
		USEBUTTON, //dont use directly, use bait instead
		YESBUTTON, //clean inventory
		SERVERNOTICE, // restart check
		OKBUTTON, //restart check
		STONE, //clean inventory
		CLOSEBUTTONX,
		BOBBERINSCALE
	};
	std::vector<int> matchingThresholds = {
		10, //scale
		151, //pie
		249, //salad
		151, //bait
		230, //logs
		200, //slot
		30, //main logo
		221, //login button
		131, //enter world button
		224, //use button
		221, //yes button
		221, //server notice
		240, //ok button
		247, //stone
		131, //close button(x)
		171  //float in scale
	};
    
	std::vector<cv::Mat> matchingTempl = {
		cv::imread("src/scale.png", cv::IMREAD_COLOR),
		cv::imread("src/chickenPie.png", cv::IMREAD_COLOR),
		cv::imread("src/salad.png", cv::IMREAD_COLOR),
		cv::imread("src/t1bait.png", cv::IMREAD_COLOR),
		cv::imread("src/logs.png", cv::IMREAD_COLOR),
		cv::imread("src/slot.png", cv::IMREAD_COLOR),
		cv::imread("src/mainLogo.png", cv::IMREAD_COLOR),
		cv::imread("src/enterButton.png", cv::IMREAD_COLOR),
		cv::imread("src/enterButton2.png", cv::IMREAD_COLOR),
		cv::imread("src/useButton.png", cv::IMREAD_COLOR),
		cv::imread("src/yesButton.png", cv::IMREAD_COLOR),
		cv::imread("src/serverNotice.png", cv::IMREAD_COLOR),
		cv::imread("src/okButton.png", cv::IMREAD_COLOR),
		cv::imread("src/stone.png", cv::IMREAD_COLOR),
		cv::imread("src/closeButton.png", cv::IMREAD_COLOR),
		cv::imread("src/coolFloat.png", cv::IMREAD_COLOR)
	};

	const std::vector<cv::TemplateMatchModes> matchingModes = {
		cv::TM_SQDIFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCORR_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_SQDIFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF_NORMED
	};
	//all this related with matchingEnum
	//////////////////////////////////////////////////////////////////////////
	

	void catchFihProcess();
	void stopCapture();
	void pressKeyMouseLeft(int KeyUpMillisec);
	void sendKeyPress(int keyCode);
	
	
	//void getDesktopMat();
	void selectAreaWithMouse(std::atomic<bool>& fihingState); //idk
	cv::Mat cropMat();
	void getMaskColorBased(cv::Mat& imgMask, objType type);
	void getImage();
	
	void catchProcess();
	cv::Mat matchingMethod(matchingEnum type, cv::Mat fullMat = cv::Mat(), cv::Rect* storedRect = nullptr);
	void TextureForDebug();
	void cleanInventory(matchingEnum matchi);
	void useBuff(matchingEnum type);
	bool checkRestart();
	//it was 1 function, but its not work in case
	void RestartingP1();
	//it was 1 function, but its not work in case
	bool okWindowCheck();
	//it was 1 function, but its not work in case
	void RestartingP2();
	cv::Mat getTemplateInTemplate(matchingEnum backTemplate, matchingEnum frontTemplate, cv::Rect& backRect);
	//get mat for both windowed and fullscreen
	void getWindowMat();

	int countThrowTime();

	
	
public:
	bool getSelectAreaState();
	void viewWindow();
	void init_g_pd3dDevice_(ID3D11Device* g_pd3dDevicee) {
		g_pd3dDevice_ = g_pd3dDevicee;
	}
	bool getTimerStatus() {
		return timerActive;
	}

	
	int duration;
	//time start for error check
	std::chrono::steady_clock::time_point clockStart;
	//time start for work timer
	std::chrono::steady_clock::time_point startWork;
	
	void mainCapture(std::atomic<bool>& fihingState, std::atomic<bool>& shouldExit);
	Vision(Config& config);
	~Vision();
	
};

