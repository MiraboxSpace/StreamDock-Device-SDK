#include "devicemanager.h"
#include <streamdockfactory.h>
#include <toolkit.h>
#include <HotspotDevice/StreamDockM18/streamdockM18.h>
#include <HotspotDevice/StreamDockN3V25/streamdockN3V25.h>

DeviceManager &DeviceManager::instance()
{
	static DeviceManager instance;
	return instance;
}

void DeviceManager::enumerator()
{
	DeviceEnumerator::instance().enumerate();
	const auto &allDevices = DeviceEnumerator::instance().currDevices();
	for (const auto &device : allDevices)
	{
		if (StreamDockFactory::instance().exist(device->_vendor_id, device->_product_id) && StreamDock::isStreamDockHidDeviceUsage(device->toPureHidDeviceInfo()) /* &&
			 StreamDock::isStreamDockHidDevice(device->toPureHidDeviceInfo())*/
		)
		{
			if (streamdocks_.find(device->_path) != streamdocks_.end()) /// exist this device and pass
				continue;
			auto dock = StreamDockFactory::instance().create(device->_vendor_id, device->_product_id, device->toPureHidDeviceInfo());
			if (!dock)
				continue;
			std::lock_guard lock(streamdocksMutex_);
			if (dock->info())
			{
				dock->info()->firmwareVersion = dock->getFirmwareVersion();
				ToolKit::print("Firmware Version:", dock->getFirmwareVersion());
			}
			streamdocks_[device->_path] = std::move(dock);
		}
	}
	ToolKit::print("[INFO] streamdock device count:", streamdocks_.size());
}

std::unordered_map<std::string, std::shared_ptr<StreamDock>> &DeviceManager::getStreamDocks()
{
	return streamdocks_;
}

DeviceManager::~DeviceManager()
{
	if (isListening_)
		stopListen();
}
