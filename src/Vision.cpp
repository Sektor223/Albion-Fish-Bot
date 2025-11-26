#include "Vision.h"

Vision::Vision(int areaRadius) : areaRadius(areaRadius) {
	if (!init) {
		initWindow();
	}
}
void Vision::startCapture(std::atomic<bool>& fihingState) {

	if (!areaSelected) {
		selectAreaWithMouse(fihingState);
	}

	while (fihingState.load())
	{
		getDesktopMat();
		getImage();
		CaptureFih();
		showImage();
	}
	if (cv::getWindowProperty(winName, cv::WND_PROP_VISIBLE) > 0) {
		stopCapture();
	}
}

void Vision::CaptureFih() 
{
	statusMessage = "start fishing";




	switch (status)
	{
	case STOPPED:
		statusMessage = "stopped";

		status = STARTED;

		break;

	case STARTED: //тут  должно быть закидывание удочки
		statusMessage = "thrown";
		pressKeyMouseLeft(1500);
		status = LOOKING;
		break;

	case LOOKING:
		statusMessage = "looking for bobber";
		if (boundRect.area() > 400) {
			status = FOUND;
			break;
		}
		break;
	case FOUND:
		statusMessage = "found and watching";
		if (boundRect.area() <= 400)
		{
			status = CATCH;
			pressKeyMouseLeft(10);
			break;
		}

		break;

	case CATCH:
		statusMessage = "catch";

		status = PULL;
		break;

	case PULL:
		statusMessage = "pull";

		status = FINISHED;
		break;

	case RELEASE:
		statusMessage = "Release";

		status = FINISHED;
		break;

	case FINISHED:
		statusMessage = "Fihing end";
		//state.fihing = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		status = STARTED;
		break;
	default:
		break;
	}


	std::this_thread::sleep_for(std::chrono::milliseconds(10));


}

void Vision::stopCapture()
{
	status = STOPPED;
	statusMessage = "Not watching";
	cv::destroyWindow(winName);
	areaSelected = false;
}

void Vision::pressKeyMouseLeft(int KeyUpMillisec) {
	//SHORT key;
	//UINT mappedKey;

	INPUT input = { 0 };
	//key = VkKeyScan('i');

	//mappedKey = MapVirtualKey(LOBYTE(key), 0);
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	//input.ki.wScan = mappedKey;
	SendInput(1, &input, sizeof(input));
	std::this_thread::sleep_for(std::chrono::milliseconds(KeyUpMillisec));
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(input));
}

bool Vision::initWindow() {
	windowDesk = GetDesktopWindow();
	init = true;
	return init;
}

void Vision::getDesktopMat() {
	HDC deviceContext = GetDC(windowDesk);
	if (!deviceContext) return;
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
}

void Vision::selectAreaWithMouse(std::atomic<bool>& fihingState) {

	POINT cursorPos;

	// ожидаем нажатие Num5 
	while (fihingState.load()) {

		if (GetAsyncKeyState(binds::fihKey) & 0x8000) {

			while (GetAsyncKeyState(binds::fihKey) & 0x8000) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			if (!fihingState.load()) break;

			GetCursorPos(&cursorPos);
			ScreenToClient(windowDesk, &cursorPos);

			// Вычисляем область вокруг курсора
			selectedArea.left = cursorPos.x - areaRadius / 2;
			selectedArea.top = cursorPos.y - areaRadius / 2;
			selectedArea.right = cursorPos.x + areaRadius / 2;
			selectedArea.bottom = cursorPos.y + areaRadius / 2;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	areaSelected = fihingState.load();
}

cv::Mat Vision::cropMat() {
	int x = max(0, selectedArea.left);
	int y = max(0, selectedArea.top);
	int width = min(fullScale.cols - x, selectedArea.right - selectedArea.left);
	int height = min(fullScale.rows - y, selectedArea.bottom - selectedArea.top);

	if (width <= 0 || height <= 0) {
		return cv::Mat();
	}

	return fullScale(cv::Rect(x, y, width, height)).clone();
}

void Vision::getMaskColorBased(cv::Mat& imgMask) {

	cv::Scalar Lower(objHSV[BOBBER][HMIN],
		objHSV[BOBBER][SMIN],
		objHSV[BOBBER][VMIN]);

	cv::Scalar Upper(objHSV[BOBBER][HMAX],
		objHSV[BOBBER][SMAX],
		objHSV[BOBBER][VMAX]);

	inRange(imgHSV, Lower, Upper, imgMask);
}

void Vision::getImage() {
	img = cropMat();

	cvtColor(img, imgHSV, cv::COLOR_BGR2HSV);

	getMaskColorBased(imgMask);

	//find countour
	findContours(imgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	//drawContours(bobberBack, contours, -1, (0, 255, 0), 3);


	//find rectangle for bobber
	for (size_t i = 0; i < contours.size(); ++i) {
		boundRect = cv::boundingRect(contours[i]);

		if (boundRect.area() > 400)
		{                                                                //&& (state.boundRect.width < 70 || state.boundRect.height < 70)
			cv::rectangle(img, boundRect.tl(), boundRect.br(), cv::Scalar(0, 0, 255), 3);
		}

	}
}

void Vision::showImage()
{
	cv::namedWindow(winName);
	cv::imshow(winName, img);
	cv::waitKey(5);
}
