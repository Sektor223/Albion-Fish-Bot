#include "Vision.h"
#include <iostream>
#include <imgui/imgui.h>
#include<chrono>
//#include<keyauth/skStr.h>


Vision::Vision(Config& config) : config(config) {
	
}
Vision::~Vision()
{
	if (bitmap) {
		DeleteObject(bitmap);
		bitmap = nullptr;
	}
	if (memoryDeviceContext) {
		DeleteDC(memoryDeviceContext);
		memoryDeviceContext = nullptr;
	}
	if (targetHWND && deviceContext) {
		ReleaseDC(targetHWND, deviceContext);
		deviceContext = nullptr;
	}

}
void Vision::mainCapture(std::atomic<bool>& fihingState, std::atomic<bool>& shouldExit) {


	if (config.windowedCapture) {
		targetHWND = FindWindowA(nullptr, "Albion Online Client");
		
	}
	else {
		targetHWND = GetDesktopWindow();
	}

	if (config.lowResolution) {
		inWaterSizeMin = 300;
		scalePosDOWN = 110;
		scalePosUP = 120;
		matchingTempl = {
		cv::imread("images/1024x768/scale.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/chickenPie.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/salad.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/t1bait.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/logs.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/slot.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/mainLogo.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/loginButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/enterWorldButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/useButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/yesButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/serverNotice.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/okButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/stone.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/closeButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1024x768/coolFloat.png", cv::IMREAD_COLOR)
		};
	}
	else {
		inWaterSizeMin = 500;
		scalePosDOWN = 135;
		scalePosUP = 150;
		matchingTempl = {
		cv::imread("images/1920x1080/scale.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/chickenPie.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/salad.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/t1bait.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/logs.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/slot.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/mainLogo.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/loginButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/enterWorldButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/useButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/yesButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/serverNotice.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/okButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/stone.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/closeButton.png", cv::IMREAD_COLOR),
		cv::imread("images/1920x1080/coolFloat.png", cv::IMREAD_COLOR)
		};
	}

	getWindowMat();
	int startRow = fullScale.rows * 0.5;
	cv::Rect roi(0, startRow, fullScale.cols, fullScale.rows - startRow);
	halved = fullScale(roi);
	if (!isAreaSelected) {
		selectAreaWithMouse(fihingState);
		SetCursorPos(itThatRemember.x, itThatRemember.y);
	}
	
	clockStart = std::chrono::high_resolution_clock::now();
	//idk why i added this
	if (config.usePie || config.useBait || config.useSalad) {
		buffsActive = true;
		if(config.usePie || config.useSalad)
		   timeFoodStart = clockStart;
	}
	else {
		buffsActive = false;
	}
	//timer until stop
	if (config.workTime != NULL) {
		startWork = std::chrono::high_resolution_clock::now();
	}
	
	
	while (fihingState.load())
	{
		if (shouldExit.load()) { break; }

			catchFihProcess();

			if (config.workTime != NULL) {
				auto endWork = std::chrono::high_resolution_clock::now();
				int workDuration = std::chrono::duration_cast<std::chrono::hours>(endWork - startWork).count();
				if (workDuration >= config.workTime) {
					fihingState = false;
					break;
				}
			}
	}
	
	stopCapture();
	
}

void Vision::catchFihProcess()
{   
	
	switch (status)
	{

	case STARTED:
		if (!timerActive) {
			timerActive = true;
			clockStart = std::chrono::high_resolution_clock::now();
		}

		
		getImage();

		statusMessage = skCrypt("thrown");
		pressKeyMouseLeft(config.throwTimeMs + (rand() % 20));
		status = LOOKING;
		break;

	case LOOKING:

		statusMessage = skCrypt("looking for bobber");
		
		getImage();

		
		//std::cout << boundRect.area() << std::endl;
		if (boundRect.area() >= inWaterSizeMin) {
			if (checkRestart()) { break; }
			status = FOUND;
			
			break;
		}
		if (checkRestart()) { break; }
		
		break;
	case FOUND:
		statusMessage = skCrypt("found and watching");
		
		
		getImage();
		
		//std::cout << boundRect.area() << std::endl;
		if (boundRect.area() < inWaterSizeMin ) {
			if (checkRestart()) { break; }
			status = CATCH;
			pressKeyMouseLeft(10);
			
		}
		
		
		
		break;

	case CATCH:
		statusMessage = skCrypt("catching");
		
		

		img = getTemplateInTemplate(SCALE, BOBBERINSCALE, scaleRect);
		

		
		if (img.empty()) {
			emptyCounter++;
			
			//magic number
			if (emptyCounter >= 7) { 
				
				
			    status = FINISHED;
				
				inputCatch.mi.dwFlags = MOUSEEVENTF_LEFTUP;
				SendInput(1, &inputCatch, sizeof(INPUT));

				emptyCounter = 0;
				
				break;
			}
		}
		else {
			catchProcess();
			emptyCounter = 0;
		}
		
		
		break;

	case FINISHED:
		clockStart = std::chrono::high_resolution_clock::now();

		status = STARTED;
		statusMessage = skCrypt("Fihing end");
		ZeroMemory(&inputCatch, sizeof(inputCatch));
		
		
		if (checkRestart()) { break; }

		//if condition for rest
		if (config.restCycles != NULL) {
			restCounter++;
			if (restCounter >= config.restCycles) {
				statusMessage = "resting";
				timerActive = false;
				std::this_thread::sleep_for(std::chrono::minutes(3));
				restCounter = 0;
				clockStart = std::chrono::high_resolution_clock::now();
				timerActive = true;
			}
			
		}
		//if condition for cleanup
		if (config.cleanCycles != NULL) {
			cleanCounter++;
			if (cleanCounter == config.cleanCycles) {
				statusMessage = skCrypt("cleaning");

				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				sendKeyPress(config.inventoryKey);
				cleanInventory(LOGS);
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				cleanInventory(STONE);
				sendKeyPress(config.inventoryKey);
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				SetCursorPos(itThatRemember.x, itThatRemember.y);

				cleanCounter = 0;
				
			}
		}
		//if condition for buffs
		if (buffsActive) {
			
			if (config.useBait) {
				baitCounter++;
				
				if (baitCounter >= 10) {
					statusMessage = skCrypt("Uses bait");
					useBuff(BAIT);
					baitCounter = 0;
				}
			}
			
			
			if (config.usePie || config.useSalad) {
				auto endFood = std::chrono::high_resolution_clock::now();
				auto durationFood = std::chrono::duration_cast<std::chrono::minutes>(endFood - timeFoodStart);
				if (durationFood >= std::chrono::minutes(30)) {
					statusMessage = skCrypt("Uses food");
					useBuff(SLOT);
					timeFoodStart = std::chrono::high_resolution_clock::now();
				}
			}
			
			
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		
		
		break;
	case SERVER_RESTART:
		statusMessage = skCrypt("Server restart detected");
		timerActive = false;
		clockStart = std::chrono::high_resolution_clock::now();

		RestartingP1();
		
		if (okWindowCheck()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			break; 
		}
		RestartingP2();
		

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		SetCursorPos(itThatRemember.x, itThatRemember.y);
		status = STARTED;
		break;
	default:
		break;
	}
	

}

void Vision::catchProcess() { 
	

	
		inputCatch.type = INPUT_MOUSE;
		inputCatch.mi.dx = 0;
		inputCatch.mi.dy = 0;
	
		

		if (clientPos.x <= scalePosDOWN && (inputCatch.mi.dwFlags != MOUSEEVENTF_LEFTDOWN)) {
			inputCatch.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
			SendInput(1, &inputCatch, sizeof(INPUT));
			//std::cout << "Mouse DOWN" << std::endl;
		}
		if (clientPos.x >= scalePosUP && (inputCatch.mi.dwFlags != MOUSEEVENTF_LEFTUP)) {
			inputCatch.mi.dwFlags = MOUSEEVENTF_LEFTUP;
			SendInput(1, &inputCatch, sizeof(INPUT));
			//std::cout << "Mouse UP" << std::endl;
		}

	}
	
void Vision::stopCapture()
{
	statusMessage = skCrypt("stopped");
	
	if (bitmap)              DeleteObject(bitmap);
	if (memoryDeviceContext) DeleteDC(memoryDeviceContext);
	if (deviceContext)       ReleaseDC(targetHWND, deviceContext);
	status = STARTED;
	isAreaSelected = false;
	buffsActive = false;
	gdiInitialized = false;
	itThatRemember = POINT();
	cleanCounter = 0;
	baitCounter = 0;
	restCounter = 0;
	cropRect = cv::Rect();
	scaleRect = cv::Rect();

}

void Vision::pressKeyMouseLeft(int KeyUpMillisec) {

	INPUT input = { 0 };

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	input.mi.dx = 0;
	input.mi.dy = 0;

	SendInput(1, &input, sizeof(input));
	std::this_thread::sleep_for(std::chrono::milliseconds(KeyUpMillisec));
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(input));
}

void Vision::sendKeyPress(int keyCode) {
	INPUT input = { 0 };
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;

	// Нажатие клавиши
	input.ki.wVk = keyCode; // Код клавиши
	input.ki.dwFlags = 0; // 0 для нажатия клавиши
	SendInput(1, &input, sizeof(INPUT));

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	// Отпускание клавиши
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
}

void Vision::selectAreaWithMouse(std::atomic<bool>& fihingState) {

	POINT cursorPos;
	statusMessage = skCrypt("waiting for area selection");
	
	while (fihingState.load()) {

		if (GetAsyncKeyState(config.fihKey) & 0x8000) {

			while (GetAsyncKeyState(config.fihKey) & 0x8000) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			if (!fihingState.load()) break;

			GetCursorPos(&cursorPos);
			ScreenToClient(targetHWND, &cursorPos);

			//cursorPos.x = 
			itThatRemember = POINT(cursorPos.x + clientRectLeft, cursorPos.y + clientRectTop);

			
			selectedArea.left = cursorPos.x - config.areaRadius / 2;
			selectedArea.top = cursorPos.y - config.areaRadius / 2;
			selectedArea.right = cursorPos.x + config.areaRadius / 2;
			selectedArea.bottom = cursorPos.y + config.areaRadius / 2;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	isAreaSelected = fihingState.load();
}

cv::Mat Vision::cropMat() {
	if (!cropRect.empty())
	{
		return fullScale(cropRect).clone();
	}
	else {
		int x = max(0, selectedArea.left);
		int y = max(0, selectedArea.top);
		int width = min(fullScale.cols - x, selectedArea.right - selectedArea.left);
		int height = min(fullScale.rows - y, selectedArea.bottom - selectedArea.top);

		if (width <= 0 || height <= 0) {
			return cv::Mat();
		}

		cropRect = cv::Rect(x, y, width, height);
		return fullScale(cropRect).clone();
	}


}

void Vision::getMaskColorBased(cv::Mat& imgMask, objType type) {

	cv::Scalar Lower(objHSV[type][HMIN],
		             objHSV[type][SMIN],
		             objHSV[type][VMIN]);

	cv::Scalar Upper(objHSV[type][HMAX],
		             objHSV[type][SMAX],
		             objHSV[type][VMAX]);

	inRange(imgHSV, Lower, Upper, imgMask);
}

void Vision::getImage() {
	
	
	getWindowMat();
	
	int countersMinVal = 40;

	
	img = cropMat();
	
	
	
	if (img.empty()) {
		
		return;
	}
	cvtColor(img, imgHSV, cv::COLOR_BGR2HSV); 

	getMaskColorBased(imgMask, BOBBER);

	
	cv::Mat blurred;
	GaussianBlur(imgMask, blurred, cv::Size(5, 5), 0);

	//find countour
	findContours(blurred, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	
	
	boundRect = cv::Rect();

	double maxArea = 0;
	int maxAreaIdx = -1;
	
	//find max area
	for (size_t i = 0; i < contours.size(); ++i) {

		double area = cv::contourArea(contours[i]);

		if (area < countersMinVal) {
			continue;
		}
		if (area > maxArea) {
			//std::cout << "area = " << area << std::endl;
			maxArea = area;
			maxAreaIdx = i;
		}
	}
	
	displayImage = img;
	//find rect for bobber
	if (maxAreaIdx >= 0) {
		boundRect = cv::boundingRect(contours[maxAreaIdx]);
		//std::cout << boundRect.area() << std::endl;
		
		
		if (boundRect.area() >= inWaterSizeMin) {
				
				
			cv::rectangle(displayImage, boundRect.tl(), boundRect.br(), cv::Scalar(0, 0, 255, 255), 3);
				
		}

		
		
	}
	

	
}

/** @brief Matches the given template, no mask.

@param type of object see matchingEnum
@return  fullScale(storedRect).clone() 

 */
cv::Mat Vision::matchingMethod(matchingEnum type, cv::Mat fullMat, cv::Rect* storedRect)
{
	if (fullMat.empty()) {
		fullMat = fullScale;
	}

	if(storedRect != nullptr && !storedRect->empty())
	{ 

		return fullMat(*storedRect).clone();
	
	}

	else {

		cv::Mat templ4chnl;
		cv::cvtColor(matchingTempl[type], templ4chnl, cv::COLOR_BGR2BGRA);
	    
		if (templ4chnl.empty()) {
			statusMessage = skCrypt("Template Not Found");
			if (storedRect != nullptr) {
				*storedRect = cv::Rect();
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			exit(1);
		}

		

		cv::Mat result;
		int result_cols = fullMat.cols - templ4chnl.cols + 1;
		int result_rows = fullMat.rows - templ4chnl.rows + 1;

		result.create(result_rows, result_cols, CV_32FC1);

		
		try {
			cv::matchTemplate(fullMat, templ4chnl, result, matchingModes[type]);
		}
		catch (const cv::Exception& e) {
			if (fullMat.type() != templ4chnl.type())
			{
				statusMessage = skCrypt("types of image not matching");
			}
			else {
				statusMessage = skCrypt("idk exeption lol");
			}

			if (storedRect != nullptr) {
				*storedRect = cv::Rect();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			exit(1);
		}
			

			
			double minVal; double maxVal;
			cv::Point minLoc, maxLoc, matchLoc;

			if  (  matchingModes[type] != cv::TM_SQDIFF_NORMED
				&& matchingModes[type] != cv::TM_CCOEFF_NORMED
				&& matchingModes[type] != cv::TM_CCORR_NORMED
				) 
			{
				normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
			}
			
			

			double thresholdValue = (double)matchingThresholds[type] / 255.0;
			threshold(result, result, thresholdValue, 1.0, cv::THRESH_BINARY);

			minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

			
			

			if (matchingModes[type] == cv::TM_SQDIFF || matchingModes[type] == cv::TM_SQDIFF_NORMED) {
				if (minLoc == cv::Point()) {
					//std::cout << "MINLOC EMPTY" << std::endl;

					return cv::Mat();
					
				}
				matchLoc = minLoc;
			}
			else {
				if (maxLoc == cv::Point()) {
					//std::cout << "MAXLOC EMPTY" << std::endl;

				    return cv::Mat();
				
				}
				matchLoc = maxLoc;
			}
			
			foundRect = cv::Rect(matchLoc, cv::Point(matchLoc.x + templ4chnl.cols, matchLoc.y + templ4chnl.rows));
			
			if (storedRect != nullptr) {
				*storedRect = foundRect;
			}

			clientPos = POINT(matchLoc.x + templ4chnl.cols / 2, matchLoc.y + templ4chnl.rows / 2);
			matchingPos = POINT(clientPos.x + clientRectLeft, clientPos.y + clientRectTop);
			


		


			
			return fullMat(foundRect).clone();
			//работает по такому же принципу как кроп мат
			



		
		
	}
	
}

void Vision::TextureForDebug()
{
	if (displayImage.empty()) {
		return;
	}
	
	
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = displayImage.cols;
	desc.Height = displayImage.rows;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = displayImage.data;
	initData.SysMemPitch = displayImage.cols * 4;

	ID3D11Texture2D* texture = nullptr;
	g_pd3dDevice_->CreateTexture2D(&desc, &initData, &texture);

	
	g_pd3dDevice_->CreateShaderResourceView(texture, nullptr, &textureSRV);
	
	texture->Release();
}

void Vision::cleanInventory(matchingEnum matchi)
{
	//start
	//sendKeyPress(config.inventoryKey);
	//std::this_thread::sleep_for(std::chrono::milliseconds(1300));
	
	getWindowMat();
	
	
	//
	//cv::Rect cleanRect = cv::Rect();
	cv::Mat cleanImage = matchingMethod(matchi);
	displayImage = cleanImage;
	if (cleanImage.empty()) {
		
		statusMessage = skCrypt("no trash found");
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		//sendKeyPress(config.inventoryKey);
		return;
	}
	
	std::this_thread::sleep_for(std::chrono::milliseconds(2500));
	//ставит курсор в центр найденного объекта
	SetCursorPos(matchingPos.x, matchingPos.y); 

	INPUT input = { 0 };

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	input.mi.dx = 0;
	input.mi.dy = 0;

	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	//левая мышь зажата
	SendInput(1, &input, sizeof(input));
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	//центр экрана/окна
	SetCursorPos(windowCenter.x, windowCenter.y);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(input));
	std::this_thread::sleep_for(std::chrono::milliseconds(1300));
	//наведение на кнопку ок
	cv::Mat buttonImg = matchingMethod(OKBUTTON);
	//надеюсь это будет работать
	SetCursorPos(matchingPos.x, matchingPos.y);
	//SetCursorPos(windowCenter.x - 70, windowCenter.y);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	pressKeyMouseLeft(10);

	//end
	std::this_thread::sleep_for(std::chrono::milliseconds(1100));
	//sendKeyPress(config.inventoryKey);
	//std::this_thread::sleep_for(std::chrono::milliseconds(1300));
	//SetCursorPos(itThatRemember.x, itThatRemember.y);
	//
}

void Vision::useBuff(matchingEnum type)
{
	
	cv::Mat buffImage;
	if (type == SLOT) {
		
		getWindowMat();
		//displayImage = fullScale;
		//std::this_thread::sleep_for(std::chrono::milliseconds(7000));
		
		cv::Mat slotImage = matchingMethod(SLOT);
		displayImage = slotImage;
		if (!slotImage.empty()) { // this condition == if slotImage found
			statusMessage = skCrypt("trying to find food");
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			//scaleRect = cv::Rect();

			sendKeyPress(config.inventoryKey);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		    
			getWindowMat();
			
			
			displayImage = buffImage;
			if (config.usePie) {
				//cv::Rect pieRect = cv::Rect();
				buffImage = matchingMethod(PIE);
				displayImage = buffImage;
				if (buffImage.empty()) {
					//statusMessage = "pie not found";
					std::this_thread::sleep_for(std::chrono::milliseconds(3000));
					sendKeyPress(config.inventoryKey);
					return;
				}

			}
			else {
				//cv::Rect saladRect = cv::Rect();
				buffImage = matchingMethod(SALAD);
				displayImage = buffImage;
				if (buffImage.empty()) {
					//statusMessage = "salad not found";
					std::this_thread::sleep_for(std::chrono::milliseconds(3000));
					sendKeyPress(config.inventoryKey);
					return;
				}

			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1500));
			SetCursorPos(matchingPos.x, matchingPos.y);
			INPUT inputs = { 0 };
			

			inputs.type = INPUT_KEYBOARD;
			inputs.ki.wVk = VK_SHIFT;

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			SendInput(1, &inputs, sizeof(INPUT));
			
			ZeroMemory(&inputs, sizeof(INPUT));

			inputs.type = INPUT_MOUSE;
			inputs.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			SendInput(1, &inputs, sizeof(INPUT));

			std::this_thread::sleep_for(std::chrono::milliseconds(300));

			ZeroMemory(&inputs, sizeof(INPUT));
			inputs.type = INPUT_MOUSE;
			inputs.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
			SendInput(1, &inputs, sizeof(INPUT));

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			ZeroMemory(&inputs, sizeof(INPUT));
			inputs.type = INPUT_KEYBOARD;
			inputs.ki.dwFlags = KEYEVENTF_KEYUP;
			inputs.ki.wVk = VK_SHIFT;
			SendInput(1, &inputs, sizeof(INPUT));

			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			sendKeyPress(config.inventoryKey);
			std::this_thread::sleep_for(std::chrono::seconds(12));
			sendKeyPress(config.foodKey);

	
			std::this_thread::sleep_for(std::chrono::milliseconds(2500));
			SetCursorPos(itThatRemember.x, itThatRemember.y);

			return;
		}

		else { //if not found just press food
			statusMessage = skCrypt("food slot not found");
			sendKeyPress(config.foodKey);
			std::this_thread::sleep_for(std::chrono::milliseconds(1100));
			return;
		}
	}

	else { //logic for bait here
		sendKeyPress(config.inventoryKey);
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		
		getWindowMat();
		
		
		//cv::Rect baitRect = cv::Rect();
		buffImage = matchingMethod(BAIT);
		displayImage = buffImage;
		if (buffImage.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			sendKeyPress(config.inventoryKey);
			return;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		SetCursorPos(matchingPos.x, matchingPos.y);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1300));
		pressKeyMouseLeft(10);
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		
		getWindowMat();
		
		

		
		//cv::Rect usebRect = cv::Rect();
		buffImage = matchingMethod(USEBUTTON);
		//displayImage = buffImage;
		if (buffImage.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			sendKeyPress(config.inventoryKey);
			return;
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		SetCursorPos(matchingPos.x, matchingPos.y);

		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		pressKeyMouseLeft(10);
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		sendKeyPress(config.inventoryKey);

		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		SetCursorPos(itThatRemember.x, itThatRemember.y);

	}

	
}

bool Vision::checkRestart() { 
	
	//cv::Rect logoRect = cv::Rect();
	cv::Mat logoImage = matchingMethod(MAINLOGO);
	if (logoImage.empty()) {
		
		return false;
	}
	else {
		status = SERVER_RESTART;
		
		return true;
	}
}

bool Vision::getSelectAreaState()
{
	return isAreaSelected;
}

void Vision::viewWindow() {
	
	if (isAreaSelected) {
		TextureForDebug();
		imguiTexture = (ImTextureID)textureSRV.Get();
	}
	else {
		imguiTexture = NULL;
	}
	
	ImGui::Begin(skCrypt("View"), NULL, config.flazhoks);
	ImGui::Text(statusMessage.c_str());
	
	ImGui::Image(imguiTexture, ImVec2(displayImage.cols, displayImage.rows));
	
	
	ImGui::TextDisabled(skCrypt("(?)"));
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip(skCrypt("if timer reaches 0 fishing stops\n resets every cycle"));
	}
	ImGui::SameLine();
	std::string errorTimer = skCrypt("error timer: ").decrypt() + std::to_string(360 - duration);
	ImGui::Text(errorTimer.c_str());

	
	
	ImGui::End();
}

void Vision::RestartingP1() { 
	
	//cv::Rect noticeRect = cv::Rect();
	cv::Mat noticeMat = matchingMethod(SERVERNOTICE);
	if (!noticeMat.empty()) {
		displayImage = noticeMat;
		statusMessage = skCrypt("notice found, sleeping for 4 minutes");
		std::this_thread::sleep_for(std::chrono::minutes(4));
	}
	
	//cv::Rect loginRect = cv::Rect();
	cv::Mat loginMat = matchingMethod(LOGINBUTTON);
	if (!loginMat.empty()) {
		displayImage = loginMat;
		statusMessage = skCrypt("Login found");
		std::this_thread::sleep_for(std::chrono::milliseconds(2050));
		SetCursorPos(matchingPos.x, matchingPos.y);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		pressKeyMouseLeft(12);
		std::this_thread::sleep_for(std::chrono::seconds(20)); //after login button clicked
	}
	
}

bool Vision::okWindowCheck() 
{
	
	
	getWindowMat();
	
	
	//cv::Rect okRect = cv::Rect();
	cv::Mat okMat = matchingMethod(OKBUTTON);
	if (!okMat.empty()) {
		displayImage = okMat;
		statusMessage = skCrypt("OK button found");
		std::this_thread::sleep_for(std::chrono::milliseconds(2050));
		SetCursorPos(matchingPos.x, matchingPos.y);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		pressKeyMouseLeft(12);
		return true;
	}
	
	return false;
	
}

void Vision::RestartingP2() 
{
	
	getWindowMat();
	
	
	//cv::Rect enterWorldRect = cv::Rect();
	cv::Mat enterWorldMat = matchingMethod(ENTERWORLDBUTTON);
	if (!enterWorldMat.empty()) {
		displayImage = enterWorldMat;
		statusMessage = skCrypt("EnterWorld found");
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		SetCursorPos(matchingPos.x, matchingPos.y);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		pressKeyMouseLeft(12);
		std::this_thread::sleep_for(std::chrono::seconds(20));

		
		getWindowMat();
		
		
		cv::Mat closeButton = matchingMethod(CLOSEBUTTONX);
		if (!closeButton.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			SetCursorPos(matchingPos.x, matchingPos.y);
			pressKeyMouseLeft(12);

		}
		/*INPUT mouseWheelUP = { 0 };
		mouseWheelUP.type = INPUT_MOUSE;
		mouseWheelUP.mi.dwFlags = MOUSEEVENTF_WHEEL;
		mouseWheelUP.mi.mouseData = WHEEL_DELTA;
		for (size_t i = 0; i < 13; i++)
		{
			SendInput(1, &mouseWheelUP, sizeof(INPUT));
			std::this_thread::sleep_for(std::chrono::milliseconds(65));
		}*/

	}
	
}

cv::Mat Vision::getTemplateInTemplate(matchingEnum backTemplate, matchingEnum frontTemplate, cv::Rect& backRect)
{
	
	getWindowMat();
	
	cv::Mat backImage = matchingMethod(backTemplate, halved, &backRect);
	if (backImage.empty()) {
		return cv::Mat();
	}
	displayImage = backImage;
	cv::Mat frontImage = matchingMethod(frontTemplate, backImage);
	if (frontImage.empty()) {
		return cv::Mat();
	}
	
	cv::rectangle(displayImage, foundRect, cv::Scalar(0, 0, 255, 255), 2);

	return frontImage;
}

void Vision::getWindowMat() {

	if (config.windowedCapture) {


		if (!gdiInitialized) {


			RECT windowRect;
			GetWindowRect(targetHWND, &windowRect);


			windowWidth = windowRect.right - windowRect.left;
			windowHeight = windowRect.bottom - windowRect.top;

			if (windowWidth <= 0 || windowHeight <= 0) return;

			RECT clientRect;
			GetClientRect(targetHWND, &clientRect);
			int borderWidth = ((windowRect.right - windowRect.left) -
				(clientRect.right - clientRect.left)) / 2;
			int titleBarHeight = (windowRect.bottom - windowRect.top) -
				(clientRect.bottom - clientRect.top) -
				borderWidth * 2;

			POINT windowTopLeft = { 0,0 };
			ClientToScreen(targetHWND, &windowTopLeft);
			clientRectLeft = windowTopLeft.x;
			clientRectTop = windowTopLeft.y;
			
			 // это надо в глобал
			windowCenter.x = windowRect.left + (windowWidth / 2);
			windowCenter.y = windowRect.top + (windowHeight / 2);

			deviceContext = GetDC(targetHWND);
			if (!deviceContext) return;

			memoryDeviceContext = CreateCompatibleDC(deviceContext);

			BITMAPINFO bmi = { 0 };
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = windowWidth;
			bmi.bmiHeader.biHeight = -windowHeight;  // Top-down DIB
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;

			void* pBits = nullptr;
			bitmap = CreateDIBSection(memoryDeviceContext, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
			SelectObject(memoryDeviceContext, bitmap);

			fullScale = cv::Mat(windowHeight, windowWidth, CV_8UC4, pBits);
			gdiInitialized = true;
		}
		BOOL result = PrintWindow(targetHWND, memoryDeviceContext, PW_RENDERFULLCONTENT);
	}
	else {
		
		clientRectLeft = 0;
		clientRectTop = 0;

		if (!gdiInitialized) {
			deviceContext = GetDC(targetHWND);
			if (!deviceContext) return;
			memoryDeviceContext = CreateCompatibleDC(deviceContext);

			windowCenter.x = screenWidth / 2;
			windowCenter.y = screenHeight / 2;

			BITMAPINFO bmi = { 0 }; //specify format by using bitmap info header
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = screenWidth;
			bmi.bmiHeader.biHeight = -screenHeight;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;

			void* pBits = nullptr;
			bitmap = CreateDIBSection(memoryDeviceContext, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
			SelectObject(memoryDeviceContext, bitmap);

			//straight use of bitmap data
			fullScale = cv::Mat(screenHeight, screenWidth, CV_8UC4, pBits);
			gdiInitialized = true;
		}

		//copy data into the bitmap
		BitBlt(memoryDeviceContext, 0, 0, screenWidth, screenHeight, deviceContext, 0, 0, SRCCOPY);
	}
}

int Vision::countThrowTime() {
	POINT cursorPos{};
	GetCursorPos(&cursorPos);
	double distance = sqrt(
		pow(cursorPos.x - windowCenter.x, 2) +
		pow(cursorPos.y - windowCenter.y, 2)
	);
	if (config.windowedCapture) {

	}
	else {

	}
}

