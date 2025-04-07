#pragma once
#include "TranSport.h"
#include "StreamDock.h"
#include <map>
#include <unordered_map>
#define AUTORECONNECT true
class DeviceManager
{
public:
	DeviceManager();
	~DeviceManager();
	std::map<std::string, std::shared_ptr<StreamDock>> enumerate();
	int listen(bool autoReconnect = false);
	void asyncListen();
	void clearListen();
	void addToPathMap(struct hid_device_info* info);

public:
	std::shared_ptr<TranSport> transport;
	std::map<std::string, std::shared_ptr<StreamDock>> m_deviceMap;

private:
	std::future<void> m_future;
	std::atomic<bool> isListening = false;
#ifdef __linux__
	std::unordered_map<std::string, std::string> m_devnodeToPath;
#endif
};
