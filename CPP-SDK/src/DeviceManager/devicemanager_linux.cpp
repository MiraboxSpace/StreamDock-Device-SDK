#include "devicemanager.h"
#if __linux__

#include <libudev.h>
#include <poll.h>
#include <thread>
#include <iostream>
#include <mutex>
#include <string>
#include <unistd.h>
#include <cstring>

void DeviceManager::listen(std::function<void(std::shared_ptr<StreamDock>)> connect_and_run)
{
	isListening_ = true;
	listener_ = std::thread([this, connect_and_run]()
		{
			struct udev* udev = udev_new();
			if (!udev) {
				ToolKit::print("[ERROR] Failed to initialize udev");
				return;
			}

			struct udev_monitor* mon = udev_monitor_new_from_netlink(udev, "udev");
			udev_monitor_filter_add_match_subsystem_devtype(mon, "hidraw", nullptr);
			udev_monitor_enable_receiving(mon);

			int fd = udev_monitor_get_fd(mon);
			struct pollfd fds = { fd, POLLIN, 0 };

			while (isListening_) {
				int ret = poll(&fds, 1, 500);
				if (ret > 0 && (fds.revents & POLLIN)) {
					struct udev_device* dev = udev_monitor_receive_device(mon);
					if (dev) {
						std::string action = udev_device_get_action(dev);
						std::string devNode = udev_device_get_devnode(dev) ? udev_device_get_devnode(dev) : "";

						if (!devNode.empty()) {
							if (action == "add") {
								/// if the device support begin
								bool canConnect = false;
								hid_device_info* devs = hid_enumerate(0x0, 0x0);
								auto cur = devs;
								while (cur) {
									DeviceEnumerator::DeviceInfo device(*cur);
									if (StreamDockFactory::instance().exist(device._vendor_id, device._product_id) && StreamDock::isStreamDockHidDeviceUsage(device.toPureHidDeviceInfo())/* &&
										StreamDock::isStreamDockHidDevice(device.toPureHidDeviceInfo())*/ && (cur->path == devNode))
										canConnect = true;
									cur = cur->next;
								}
								if (!canConnect) {
									hid_free_enumeration(devs);
									continue;
								}
								hid_free_enumeration(devs);
								/// if the device support end
								enumerator();  // 重新枚举设备
								std::lock_guard<std::mutex> lock(streamdocksMutex_);
								auto it = getStreamDocks().find(devNode);
								if (it != getStreamDocks().end() && connect_and_run) {
									connect_and_run(it->second);
								}
								ToolKit::print("[+] HID Device Added: ", devNode);
							}
							else if (action == "remove") {
								std::lock_guard<std::mutex> lock(streamdocksMutex_);
								auto& docks = getStreamDocks();
								if (docks.find(devNode) != docks.end()) {
									docks.erase(devNode);
									ToolKit::print("[-] HID Device Removed: ", devNode);
								}
							}
						}
						udev_device_unref(dev);
					}
				}
			}

			udev_monitor_unref(mon);
			udev_unref(udev); });
}

void DeviceManager::stopListen()
{
	isListening_ = false;
	if (listener_.joinable())
		listener_.join();
}

#endif
