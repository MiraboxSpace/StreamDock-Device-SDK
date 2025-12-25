#include "devicemanager.h"
#include <thread>
#include <chrono>
#include <unordered_set>
#include <algorithm>
#include <sstream>

#ifdef __APPLE__

void DeviceManager::listen(std::function<void(std::shared_ptr<StreamDock>)> connect_and_run)
{
	isListening_ = true;
	listener_ = std::thread([this, connect_and_run]()
		{
			// Store initial device paths
			std::unordered_set<std::string> currentDevicePaths;

			// Initialize current device list
			{
				std::lock_guard<std::mutex> lock(streamdocksMutex_);
				for (const auto& pair : streamdocks_)
				{
					currentDevicePaths.insert(pair.first);
				}
			}

			// Polling interval in milliseconds (same as Python SDK)
			const int pollingIntervalMs = 2000;

			while (isListening_)
			{
				try
				{
					// Enumerate all HID devices
					DeviceEnumerator::instance().enumerate();
					const auto& allDevices = DeviceEnumerator::instance().currDevices();

					// Collect paths of supported StreamDock devices
					std::unordered_set<std::string> newDevicePaths;

					for (const auto& device : allDevices)
					{
						// Check if this is a supported StreamDock device
						if (StreamDockFactory::instance().exist(device->_vendor_id, device->_product_id) &&
							StreamDock::isStreamDockHidDeviceUsage(device->toPureHidDeviceInfo()))
						{
							newDevicePaths.insert(device->_path);
						}
					}

					// Check for added devices
					std::unordered_set<std::string> addedDevices;
					for (const auto& path : newDevicePaths)
					{
						if (currentDevicePaths.find(path) == currentDevicePaths.end())
						{
							addedDevices.insert(path);
						}
					}

					// Check for removed devices
					std::unordered_set<std::string> removedDevices;
					for (const auto& path : currentDevicePaths)
					{
						if (newDevicePaths.find(path) == newDevicePaths.end())
						{
							removedDevices.insert(path);
						}
					}

					// Handle removed devices first
					if (!removedDevices.empty())
					{
						std::lock_guard<std::mutex> lock(streamdocksMutex_);
						for (const auto& path : removedDevices)
						{
							auto it = streamdocks_.find(path);
							if (it != streamdocks_.end())
							{
								std::ostringstream oss;
								oss << "[-] HID Device Removed: " << path;
								ToolKit::print(oss.str());
								streamdocks_.erase(it);
							}
						}
					}

					// Handle added devices
					if (!addedDevices.empty())
					{
						// Re-enumerate to create device objects
						enumerator();

						std::lock_guard<std::mutex> lock(streamdocksMutex_);
						for (const auto& path : addedDevices)
						{
							auto it = streamdocks_.find(path);
							if (it != streamdocks_.end())
							{
								std::ostringstream oss;
								oss << "[+] HID Device Added: " << path;
								ToolKit::print(oss.str());

								// Call the callback if provided
								if (connect_and_run)
								{
									connect_and_run(it->second);
								}
							}
						}
					}

					// Update current device list
					currentDevicePaths = newDevicePaths;

				}
				catch (const std::exception& e)
				{
					std::ostringstream oss;
					oss << "[ERROR] macOS polling error: " << e.what();
					ToolKit::print(oss.str());
				}
				catch (...)
				{
					ToolKit::print("[ERROR] Unknown macOS polling error");
				}

				// Wait before next polling cycle
				std::this_thread::sleep_for(std::chrono::milliseconds(pollingIntervalMs));
			}
		});
}

void DeviceManager::stopListen()
{
	isListening_ = false;
	if (listener_.joinable())
		listener_.join();
}

#endif
