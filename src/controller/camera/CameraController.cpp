#include "CameraController.h"

CameraController::CameraController(int camerasFps)
{
	this->optitrackCamera = new OptitrackCamera();
	this->capture = new Capture();
	this->safeFrame = FramesPacket();
	this->shouldUpdateSafeFrame = false;
	this->shouldLoopThread = false;
	this->shouldRecord = false;
	this->shouldCapture = false;
	this->camerasFps = camerasFps;
}

bool CameraController::startCameras(CaptureMode mode)
{
	if (optitrackCamera->startCameras(mode.toOptitrackMode()))
	{
		if (capture != nullptr)
		{
			delete capture;
		}		
		capture = new Capture();

		shouldLoopThread = true;
		thread camerasThread = thread(&CameraController::cameraLoop, this);
		camerasThread.detach();
		return true;
	}
	
	return false;
}

void CameraController::cameraLoop()
{
	bool shouldKeepPrevFrame = false;
	FramesPacket* currentFrame = nullptr;

	while (shouldLoopThread)
	{
		int milisecondsToSleep = (int)(1.0 / camerasFps) * 1000;
		std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now() + std::chrono::milliseconds(milisecondsToSleep);

		if (!shouldKeepPrevFrame && currentFrame != nullptr)
		{
			delete currentFrame;
		}

		currentFrame = optitrackCamera->captureFramesPacket();

		if (currentFrame->isEmpty())
		{
			delete currentFrame;
			currentFrame = nullptr;
			continue;
		}

		shouldKeepPrevFrame = shouldRecord || shouldCapture;

		if (shouldRecord)
		{
			capture->addToCaptureRecording(currentFrame);
		}

		if (shouldCapture)
		{
			capture->addToCaptureFrame(currentFrame);
			shouldCapture = false;
		}

		if (shouldUpdateSafeFrame)
		{
			safeFrame = FramesPacket(currentFrame);
			shouldUpdateSafeFrame = false;
		}

		std::this_thread::sleep_until(timePoint);
	}
}

void CameraController::stopCameras()
{
	shouldLoopThread = false;
	optitrackCamera->stopCameras();
}

void CameraController::startRecording()
{
	shouldRecord = true;
}

void CameraController::stopRecording()
{
	shouldRecord = false;
}

void CameraController::captureFrame()
{
	shouldCapture = true;
}

void CameraController::updateSafeFrame()
{
	shouldUpdateSafeFrame = true;
}

FramesPacket CameraController::getSafeFrame()
{
	return safeFrame;
}

Capture* CameraController::getCapture()
{
	return capture;
}

int CameraController::getCamerasFps()
{
	return camerasFps;
}