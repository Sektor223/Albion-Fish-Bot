#pragma once
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include <Windows.h>
#include<atomic>
#include"Utility.h"

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
		STOPPED,
		STARTED,
		LOOKING,
		FOUND,
		CATCH,
		PULL,
		RELEASE,
		FINISHED
	};


	int areaRadius; //передаётся в конструктор

	//если буду использовать несколько объектов класса надо будет пересмотреть
	static inline HWND windowDesk = nullptr;
	static inline cv::Mat img = cv::Mat();
	static inline cv::Mat fullScale = cv::Mat();
	static inline bool init = false;
	static inline bool areaSelected = false;
	static inline Status status = STOPPED;

	cv::Mat	imgHSV, imgMask;
	std::vector<std::vector<cv::Point>> contours;
	::RECT selectedArea = { 0 };
	cv::Rect boundRect = cv::Rect();

	//add new objects here
	const std::vector<std::vector<int>> objHSV = {
		//hmin, hmax, smin, smax, vmin, vmax
			{0, 14, 80, 207, 124, 255} //BOBBER

	};

	const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	//mouse related
	void CaptureFih();
	void stopCapture();
	void pressKeyMouseLeft(int KeyUpMillisec);

	//vision related
	bool initWindow();
	void getDesktopMat();
	void selectAreaWithMouse(std::atomic<bool>& fihingState); //idk
	cv::Mat cropMat();
	void getMaskColorBased(cv::Mat& imgMask);
	void getImage();
	void showImage();

public:
	static inline std::string statusMessage = "never started";
	const std::string winName = "Debug Window";
	void startCapture(std::atomic<bool>& fihingState);
	Vision(int areaRadius);
};

