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
				if (dock->getFirmwareVersion().find("V2.M18") != std::string::npos)
				{
					if (StreamDockM18 *dev = dynamic_cast<StreamDockM18 *>(dock.get()))
					{
						dev->changeV2Mode();
						std::wcout << "StreamDockM18V2: " << dev->info()->serialNumber << std::endl;
					}
				}
				else if (dock->getFirmwareVersion().find("V25.M18") != std::string::npos)
				{
					if (StreamDockM18 *dev = dynamic_cast<StreamDockM18 *>(dock.get()))
					{
						dev->changeV2Mode();
						std::wcout << "StreamDockM18V25: " << dev->info()->serialNumber << std::endl;
					}
				}
				else if (dock->getFirmwareVersion().find("M18") != std::string::npos)
					std::wcout << "StreamDockM18: " << dock->info()->serialNumber << std::endl;
				else if (dock->getFirmwareVersion().find("V25.N3") != std::string::npos)
				{
					if (StreamDockN3V25 *dev = dynamic_cast<StreamDockN3V25 *>(dock.get()))
					{
						dev->changeV2Mode();
						std::wcout << "StreamDockN3V25: " << dev->info()->serialNumber << std::endl;
					}
				}
				else if (dock->getFirmwareVersion().find("V3.N3") != std::string::npos)
					std::wcout << "StreamDockN3V3: " << dock->info()->serialNumber << std::endl;
				else if (dock->getFirmwareVersion().find("N3") != std::string::npos)
					std::wcout << "StreamDockN3: " << dock->info()->serialNumber << std::endl;
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
