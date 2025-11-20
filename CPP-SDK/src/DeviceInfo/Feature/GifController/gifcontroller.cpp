#include "gifcontroller.h"
#include <iostream>
#include <toolkit.h>

GifController::GifController(StreamDock* instance)
	: _instance(instance)
{
	startWorkerThread();
}

GifController::~GifController()
{
	stopWorkerThread();
}

void GifController::setKeyGifFile(const std::string& gifPath, uint8_t keyValue, uint16_t gifDelay)
{
	if (!_instance)
		return;
	if (_instance->outOfRange(keyValue))
	{
		ToolKit::print("[ERROR] Key value out of range: ", static_cast<int>(keyValue));
		return;
	}
	if (!_instance->_transport || !_instance->_transport->canWrite() || !_instance->_feature->isDualDevice) return;
	auto gifStream = StreamDock::readGifToStream(gifPath, _instance->_encoder, *(_instance->getKyImgHelper(keyValue)));
	std::vector<std::string> _gifStream;
	for (const auto& frame : gifStream)
	{
		_gifStream.emplace_back(std::string(frame.begin(), frame.begin() + frame.size()));
	}
	std::lock_guard<std::mutex> lock(_gifMutex);
	_gifMap.insert_or_assign(keyValue, GifStreamStatus{ _gifStream, 0, gifDelay });
}

void GifController::setKeyGifStream(const std::vector<std::string>& gifStream, uint8_t keyValue, uint16_t gifDelay)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice)
	{
		std::lock_guard<std::mutex> lock(_gifMutex);
		_gifMap[keyValue] = GifStreamStatus{ gifStream, 0, gifDelay };
	}
}

void GifController::setKeyGifStream(const std::vector<std::vector<uint8_t>>& gifStream, uint8_t keyValue, uint16_t gifDelay)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice)
	{
		std::vector<std::string> _gifStream;
		for (const auto& frame : gifStream)
		{
			_gifStream.push_back(std::string(frame.begin(), frame.begin() + frame.size()));
		}
		setKeyGifStream(_gifStream, keyValue);
	}
}

void GifController::setBackgroundGifFile(const std::string& gifPath, int16_t background_place_x, uint16_t background_place_y, uint16_t gifDelay, uint8_t FBlayer)
{
	if (!_instance)
		return;
	if (!_instance->_transport || !_instance->_transport->canWrite() || !_instance->_feature->isDualDevice || !_instance->_feature->supportBackGroundGif) return;
	auto gifStream = StreamDock::readGifToStream(gifPath, _instance->_encoder, *(_instance->getBackgroundGifHelper()));
	std::vector<std::string> _gifStream;
	for (const auto& frame : gifStream)
	{
		_gifStream.emplace_back(std::string(frame.begin(), frame.begin() + frame.size()));
	}
	_background_place_x = background_place_x;
	_background_place_y = background_place_y;
	std::lock_guard<std::mutex> lock(_gifMutex);
	_gifMap.insert_or_assign(0, GifStreamStatus{ _gifStream, 0, gifDelay });   /// 0 号位置给背景 gif
}

void GifController::setBackgroundGifStream(const std::vector<std::string>& gifStream, int16_t background_place_x, uint16_t background_place_y, uint16_t gifDelay, uint8_t FBlayer)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice && _instance->_feature->supportBackGroundGif)
	{
		_background_place_x = background_place_x;
		_background_place_y = background_place_y;
		std::lock_guard<std::mutex> lock(_gifMutex);
		_gifMap[0] = GifStreamStatus{ gifStream, 0, gifDelay };
	}
}

void GifController::setBackgroundGifStream(const std::vector<std::vector<uint8_t>>& gifStream, uint16_t background_place_x, uint16_t background_place_y, uint16_t gifDelay, uint8_t FBlayer)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice && _instance->_feature->supportBackGroundGif)
	{
		std::vector<std::string> _gifStream;
		for (const auto& frame : gifStream)
		{
			_gifStream.push_back(std::string(frame.begin(), frame.begin() + frame.size()));
		}
		setBackgroundGifStream(_gifStream, background_place_x, background_place_y, gifDelay, FBlayer);
	}
}

void GifController::clearBackgroundGifStream(uint8_t clearPostion)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice && _instance->_feature->supportBackGroundGif)
		clearBackgroundGifFileFrame(clearPostion);
}

void GifController::clearKeyGif(uint8_t keyValue)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice)
	{
		std::lock_guard<std::mutex> lock(_gifMutex);
		_gifMap.erase(keyValue);
	}
}

void GifController::clearBackgroundyGif()
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice && _gifMap.find(0) != _gifMap.end())
	{
		std::lock_guard<std::mutex> lock(_gifMutex);
		_gifMap.erase(0);
	}
}

void GifController::startGifLoop()
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice)
		_gifLoopEnabled = true;
	_gifCv.notify_all();
}

void GifController::stopGifLoop()
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice)
		_gifLoopEnabled = false;
}

void GifController::gifWorkLoop()
{
	if (!_instance || !_instance->canTransportWrite() || !_instance->_feature->isDualDevice) return;
	while (_running)
	{
		{
			std::unique_lock<std::mutex> lock(_gifMutex);
			_gifCv.wait(lock, [this]
				{ return !_running || _gifLoopEnabled; });

			if (!_instance->canTransportWrite())
				_running = false;
			if (!_running)
				break;

			if (_gifMap.empty()) continue;

			for (auto& [index, _gif] : _gifMap)
			{
				auto& gifFrames = _gif.gifFrames;
				auto& currentFrame = _gif.currentFrame;
				const auto gifDelay = _gif.gifDelay;
				currentFrame += _baseGifDelayMs / gifDelay;
				if (currentFrame >= gifFrames.size())
					currentFrame = 0;
				if (gifFrames.empty())
					continue;
				if (index != 0) {
					_instance->setKeyImgFileStream(gifFrames[currentFrame], index);
				}
				else if (index == 0 &&
					_background_place_x + _instance->getBackgroundGifHelper()->_width <= _instance->getBgImgHelper()->_width &&
					_background_place_y + _instance->getBackgroundGifHelper()->_height <= _instance->getBgImgHelper()->_height) {
					setBackgroundGifFileFrame(gifFrames[currentFrame],
						_instance->getBackgroundGifHelper()->_width, _instance->getBackgroundGifHelper()->_height,
						_background_place_x, _background_place_y);
				}
			}
		}
		_instance->wakeupScreen();
		_instance->refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(_baseGifDelayMs));
	}
	ToolKit::print("[INFO] exit gif worker loop");
}

bool GifController::gifWorkLoopStatus()
{
	return _gifLoopEnabled;
}

void GifController::startWorkerThread()
{
	_running = true;
	_gifThread = std::thread(&GifController::gifWorkLoop, this);
}

void GifController::stopWorkerThread()
{
	_running = false;
	_gifCv.notify_all();
	if (_gifThread.joinable())
		_gifThread.join();
}


void GifController::setBackgroundGifFileFrame(const std::string& stream, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t FBlayer)
{
	if (!_instance)
		return;
	if (!_instance->canTransportWrite())
	{
		ToolKit::print("[ERROR] Transport is not running.");
		return;
	}
	if (_instance->_feature->isDualDevice && _instance->_feature->supportBackGroundGif)
	{
		if (!StreamDock::isJpegData(stream))
		{
			ToolKit::print("[ERROR] Invalid JPEG data.");
			return;
		}
		_instance->_transport->setBackgroundFrameStream(stream, width, height, x, y);
	}
}

void GifController::clearBackgroundGifFileFrame(uint8_t clearPostion)
{
	if (!_instance)
		return;
	if (!_instance->canTransportWrite())
	{
		ToolKit::print("[ERROR] Transport is not running.");
		return;
	}
	if (_instance->_feature->isDualDevice && _instance->_feature->supportBackGroundGif)
	{
		_instance->_transport->clearBackgroundFrameStream(clearPostion);
	}
}
