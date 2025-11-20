#include "heartbeat.h"
#include <toolkit.h>

HeartBeat::HeartBeat(StreamDock* instance) 
	: _instance(instance)
{
	startWorkerThread();
}

HeartBeat::~HeartBeat() 
{
	stopWorkerThread();
}

void HeartBeat::startHeartBeatLoop()
{
	if (!_instance)
		return;
	if (_instance->_transport)
		_HeartBeatLoopEnabled = true;
	_heartbeatCv.notify_all();
} 

void HeartBeat::stopHeartBeatLoop()
{
	if (!_instance)
		return;
	if (_instance->_transport)
		_HeartBeatLoopEnabled = false;
}

void HeartBeat::heartBeatLoop()
{
	if (!_instance || !_instance->canTransportWrite()) return;

	ToolKit::print("[INFO] heart beat worker started");

	while (_running)
	{
		{
			std::unique_lock<std::mutex> lock(_heartbeatMutex);
			_heartbeatCv.wait_for(lock, std::chrono::seconds(HEART_BEAT_TIME), [this] {
				return !_running;
				});

			if (!_running || !_instance || !_instance->_transport->canWrite())
				break;
		}

		if (!_HeartBeatLoopEnabled) continue;
		/// 发送心跳包
		_instance->heartbeat();
	}
	ToolKit::print("[INFO] exit heart beat worker loop");
}

void HeartBeat::startWorkerThread() 
{
	_running = true;
	_heartbeatThread = std::thread(&HeartBeat::heartBeatLoop, this);
}
void HeartBeat::stopWorkerThread() 
{
	_running = false;
	_heartbeatCv.notify_all();
	if (_heartbeatThread.joinable())
		_heartbeatThread.join();
}