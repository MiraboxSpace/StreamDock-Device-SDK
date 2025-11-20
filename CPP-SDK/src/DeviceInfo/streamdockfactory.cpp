#include "streamdockfactory.h"

StreamDockFactory& StreamDockFactory::instance()
{
	static StreamDockFactory _instance;
	return _instance;
}

void StreamDockFactory::registerDevice(uint16_t vid, uint16_t pid, Creator creator)
{
	deviceMap[{vid, pid}] = creator;
}

bool StreamDockFactory::exist(uint16_t vid, uint16_t pid) const
{
	return deviceMap.find({ vid, pid }) != deviceMap.end();
}

std::unique_ptr<StreamDock> StreamDockFactory::create(uint16_t vid, uint16_t pid, const hid_device_info& device_info)
{
	auto it = deviceMap.find({ vid, pid });
	if (it != deviceMap.end())
	{
		return it->second(device_info);
	}
	return nullptr;
}
