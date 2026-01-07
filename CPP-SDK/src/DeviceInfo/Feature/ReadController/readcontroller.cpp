#include "readcontroller.h"
#include <future>
#include <iomanip>
#include <toolkit.h>

ReadController::ReadController(StreamDock *instance)
	: _instance(instance)
{
	startWorkerThread();
}

ReadController::~ReadController()
{
	stopWorkerThread();
}

std::vector<uint8_t> ReadController::read(int32_t timeoutMs)
{
	if (!_instance)
		return {};
	if (!_instance->_transport)
	{
		ToolKit::print("[ERROR] Transport is not running or cannot write.");
		return {};
	}
	int64_t length = -1;
	std::vector<uint8_t> response(2048); // Max response length is 2048 bytes
	_instance->_transport->read(response.data(), reinterpret_cast<size_t *>(&length), timeoutMs);
	if (length > 0)
	{
		response.resize(length);
		// ToolKit::printHexBuffer(response);
		return response;
	}
	else if (length == -1)
	{ /// disconnect or other abort
		stopWorkerThread();
		return {};
	}
	else
	{
		// ToolKit::print("No data received.");
	}
	return {};
}

void ReadController::registerReadCallback(uint8_t realValue, IReadController::ReadCallback callback, RegisterEvent event, bool callbackAsync)
{
	std::lock_guard<std::mutex> lock(_readMutex);
	_readCallbackMap[{realValue, event}] = CallbackStructure{callback, callbackAsync};
}

void ReadController::unregisterReadCallback(uint8_t realValue, RegisterEvent event)
{
	std::lock_guard<std::mutex> lock(_readMutex);
	_readCallbackMap.erase({realValue, event});
}

void ReadController::registerRawReadCallback(IReadController::RawReadCallback callback, bool callbackAsync)
{
	std::lock_guard<std::mutex> lock(_readMutex);
	_rawReadCallback = {callback, callbackAsync};
}

void ReadController::unregisterRawReadCallback()
{
	std::lock_guard<std::mutex> lock(_readMutex);
	_rawReadCallback = {nullptr, false};
}

void ReadController::startReadLoop()
{
	if (!_instance)
		return;
	if (_instance->_transport)
		_readLoopEnabled = true;
	_readCv.notify_all();
}

void ReadController::stopReadLoop()
{
	if (!_instance)
		return;
	if (_instance->_transport)
		_readLoopEnabled = false;
}

void ReadController::readLoop()
{
	if (!_instance || !_instance->_transport)
		return;
	while (_running)
	{
		{
			std::unique_lock<std::mutex> lock(_readMutex);
			_readCv.wait(lock, [this]
						 { return !_running || _readLoopEnabled; });

			if (!_instance->_transport->canWrite())
				_running = false;
			if (!_running)
				break;
		}

		std::vector<uint8_t> response = read(READ_LOOP_TIMEOUT);
		if (response.empty())
			continue;
		if (response.size() < 64)
		{
			ToolKit::print("[ERROR] Received response is too short.");
			continue;
		}
		/// Raw data callback handling
		if (_rawReadCallback.callback)
		{
			RawReadCallback callback = _rawReadCallback.callback;
			if (_rawReadCallback.async_)
				std::thread([callback, response]
							{ callback(response); })
					.detach();
			else
				callback(response);
		}
		bool isK1Pro = (_instance->_info->originType == DeviceOriginType::K1Pro);
		/// Registered event callback handling
		bool flag = isK1Pro ? response[0] == 0x04 &&response[1] == 0x41 && response[2] == 0x43 && response[3] == 0x4B && response[6] == 0x4F && response[7] == 0x4B : response[0] == 0x41 && response[1] == 0x43 && response[2] == 0x4B && response[5] == 0x4F && response[6] == 0x4B; // Check response header
		if (!flag)
			continue; // If the response header doesn't match, skip processing

		// K1Pro uses response[10] and response[11]; other devices use response[9] and response[10]
		size_t readValueOffset = isK1Pro ? 10 : 9;
		size_t eventValueOffset = isK1Pro ? 11 : 10;

		uint8_t readValue = response[readValueOffset]; // Get key value
		uint8_t realValue = 0xFF;					   // Actual registered device key value
		for (const auto &ite : _instance->_readValueMap)
		{
			if (ite.second == readValue)
			{
				realValue = ite.first;
				break;
			}
		}
		if (realValue == 0xFF)
			continue;										 // If no registered device key value is found, skip processing
		uint8_t readEventValue = response[eventValueOffset]; // Get event value
		RegisterEvent triggeredEvent = _instance->dispatchEvent(readValue, readEventValue);
		{
			std::unique_lock<std::mutex> lock(_readMutex);
			auto exactIt = _readCallbackMap.find({realValue, triggeredEvent});
			if (exactIt != _readCallbackMap.end() && exactIt->second.callback)
			{ // Call the exact-match event callback
				if (!(exactIt->second.async_))
				{
					exactIt->second.callback();
					std::cout.flush();  // Flush output immediately
				}
				else
					std::thread([exactIt]
								{ exactIt->second.callback(); })
						.detach();
			}
			auto anyIt = _readCallbackMap.find({realValue, RegisterEvent::EveryThing});
			if (anyIt != _readCallbackMap.end() && anyIt->second.callback)
			{
				if (!(anyIt->second.async_))
					anyIt->second.callback();
				else
					std::thread([anyIt]
								{ anyIt->second.callback(); })
						.detach();
			}
		}
	}
	ToolKit::print("[INFO] exit read worker loop");
}

void ReadController::startWorkerThread()
{
	_running = true;
	_readThread = std::thread(&ReadController::readLoop, this);
}

void ReadController::stopWorkerThread()
{
	_running = false;
	_readCv.notify_all();
	if (_readThread.joinable())
		_readThread.join();
}
