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

void GifController::setKeyGifFile(const std::string& gifPath, uint8_t keyValue)
{
	if (!_instance)
		return;
	if (_instance->outOfRange(keyValue))
	{
		ToolKit::print("[ERROR] Key value out of range: ", static_cast<int>(keyValue));
		return;
	}
	if (!_instance->_transport || !_instance->_transport->canWrite() || !_instance->_feature->isDualDevice) return;

	// Use the new method to read frames and delays
	auto gifDataWithDelays = StreamDock::readGifWithDelays(gifPath, _instance->_encoder, *(_instance->getKyImgHelper(keyValue)));

	std::vector<std::string> gifFrames;
	std::vector<uint16_t> frameDelays;
	gifFrames.reserve(gifDataWithDelays.size());
	frameDelays.reserve(gifDataWithDelays.size());

	for (const auto& frame : gifDataWithDelays)
	{
		gifFrames.emplace_back(std::string(frame.encodedData.begin(), frame.encodedData.end()));
		frameDelays.push_back(frame.delayMs);
	}

	std::lock_guard<std::mutex> lock(_gifMutex);
	_gifMap.insert_or_assign(keyValue, GifStreamStatus{ gifFrames, frameDelays, 0, 0 });
}

void GifController::setKeyGifStream(const std::vector<std::string>& gifStream, const std::vector<uint16_t>& frameDelays, uint8_t keyValue)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice)
	{
		std::lock_guard<std::mutex> lock(_gifMutex);
		_gifMap[keyValue] = GifStreamStatus{ gifStream, frameDelays, 0, 0 };
	}
}

void GifController::setKeyGifStream(const std::vector<std::vector<uint8_t>>& gifStream, const std::vector<uint16_t>& frameDelays, uint8_t keyValue)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice)
	{
		std::vector<std::string> _gifStream;
		_gifStream.reserve(gifStream.size());  // Pre-allocate memory to reduce reallocations
		for (auto&& frame : gifStream)
		{
			_gifStream.push_back(std::string(frame.begin(), frame.end()));
		}
		setKeyGifStream(_gifStream, frameDelays, keyValue);
	}
}

void GifController::setBackgroundGifFile(const std::string& gifPath, int16_t background_place_x, uint16_t background_place_y, uint8_t FBlayer)
{
	if (!_instance)
		return;
	if (!_instance->_transport || !_instance->_transport->canWrite() || !_instance->_feature->isDualDevice || !_instance->_feature->supportBackGroundGif) return;

	// Use the new method to read frames and delays
	auto gifDataWithDelays = StreamDock::readGifWithDelays(gifPath, _instance->_encoder, *(_instance->getBackgroundGifHelper()));

	std::vector<std::string> gifFrames;
	std::vector<uint16_t> frameDelays;
	gifFrames.reserve(gifDataWithDelays.size());
	frameDelays.reserve(gifDataWithDelays.size());

	for (const auto& frame : gifDataWithDelays)
	{
		gifFrames.emplace_back(std::string(frame.encodedData.begin(), frame.encodedData.end()));
		frameDelays.push_back(frame.delayMs);
	}

	_background_place_x = background_place_x;
	_background_place_y = background_place_y;
	std::lock_guard<std::mutex> lock(_gifMutex);
	_gifMap.insert_or_assign(0, GifStreamStatus{ gifFrames, frameDelays, 0, 0 });   /// Index 0 reserved for background GIF
}

void GifController::setBackgroundGifStream(const std::vector<std::string>& gifStream, const std::vector<uint16_t>& frameDelays, int16_t background_place_x, uint16_t background_place_y, uint8_t FBlayer)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice && _instance->_feature->supportBackGroundGif)
	{
		_background_place_x = background_place_x;
		_background_place_y = background_place_y;
		std::lock_guard<std::mutex> lock(_gifMutex);
		_gifMap[0] = GifStreamStatus{ gifStream, frameDelays, 0, 0 };
	}
}

void GifController::setBackgroundGifStream(const std::vector<std::vector<uint8_t>>& gifStream, const std::vector<uint16_t>& frameDelays, uint16_t background_place_x, uint16_t background_place_y, uint8_t FBlayer)
{
	if (!_instance)
		return;
	if (_instance->_transport && _instance->_transport->canWrite() && _instance->_feature->isDualDevice && _instance->_feature->supportBackGroundGif)
	{
		std::vector<std::string> _gifStream;
		_gifStream.reserve(gifStream.size());  // Pre-allocate memory to reduce reallocations
		for (auto&& frame : gifStream)
		{
			_gifStream.push_back(std::string(frame.begin(), frame.end()));
		}
		setBackgroundGifStream(_gifStream, frameDelays, background_place_x, background_place_y, FBlayer);
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

	// Get start time
	auto lastTime = std::chrono::steady_clock::now();

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

			if (_gifMap.empty()) {
				lock.unlock();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				lastTime = std::chrono::steady_clock::now(); // Reset timer to prevent large delta
				continue;
			}
		}

		// Compute elapsed time for this frame (microseconds)
		auto currentTime = std::chrono::steady_clock::now();
		auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime).count();
		lastTime = currentTime;

		// Collect GIF frames that need updates
		std::vector<std::pair<uint8_t, size_t>> framesToUpdate;
		{
			std::lock_guard<std::mutex> lock(_gifMutex);
			for (auto& [index, _gif] : _gifMap)
			{
				if (_gif.gifFrames.empty() || _gif.frameDelays.empty())
					continue;

				// Accumulate time
				_gif.accumulatedTime += elapsedUs;

				// Use the current frame delay
				uint64_t currentFrameDelay = static_cast<uint64_t>(_gif.frameDelays[_gif.currentFrame]) * 1000;

				// Check if the current frame delay has been exceeded
				if (_gif.accumulatedTime >= currentFrameDelay)
				{
					// Update frame index
					_gif.currentFrame = (_gif.currentFrame + 1) % _gif.gifFrames.size();
					// Subtract the current frame delay (keep the remainder for precise timing)
					_gif.accumulatedTime -= currentFrameDelay;
					framesToUpdate.push_back({index, _gif.currentFrame});
				}
			}
		}

		// If there are frames to update, perform USB transfer
		if (!framesToUpdate.empty())
		{
			std::lock_guard<std::mutex> lock(_gifMutex);
			for (const auto& [index, frameIndex] : framesToUpdate)
			{
				auto it = _gifMap.find(index);
				if (it == _gifMap.end() || it->second.gifFrames.empty())
					continue;

				const auto& gifFrames = it->second.gifFrames;

				if (index != 0) {
					_instance->setKeyImgFileStream(gifFrames[frameIndex], index);
				}
				else if (index == 0 &&
					_background_place_x + _instance->getBackgroundGifHelper()->_width <= _instance->getBgImgHelper()->_width &&
					_background_place_y + _instance->getBackgroundGifHelper()->_height <= _instance->getBgImgHelper()->_height) {
					setBackgroundGifFileFrame(gifFrames[frameIndex],
						_instance->getBackgroundGifHelper()->_width, _instance->getBackgroundGifHelper()->_height,
						_background_place_x, _background_place_y);
				}
			}

			// Batch refresh - refresh once per batch
			_instance->refresh();
		}

		// Brief sleep to reduce CPU usage (~3 ms)
		std::this_thread::sleep_for(std::chrono::milliseconds(3));
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
