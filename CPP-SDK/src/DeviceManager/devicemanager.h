/**
 * @file DeviceManager.h
 * @brief Singleton class for managing StreamDock devices with enumeration and plug/unplug monitoring.
 *
 * Main Features:
 * - Enumerate currently connected devices
 * - Start a background thread to listen for device events (cross-platform)
 * - Manage and provide access to all active StreamDock instances
 */
#pragma once
#ifdef _WIN32
#include <windows.h>
#include <setupapi.h>
#include <Dbt.h>
#include <regex>
#endif
#include <memory>
#include <unordered_map>
#include <streamdock.h>
#include <streamdockfactory.h>
#include <toolkit.h>
#include "./DeviceEnumerator/deviceenumerator.h"

 /**
  * @class DeviceManager
  * @brief Singleton class for managing StreamDock device connections.
  *
  * Responsible for:
  * - Enumerating currently connected StreamDock devices
  * - Starting a background thread to monitor device plug/unplug events
  * - Holding references to active device instances
  *
  * Thread-safe access. Copy operations are disabled.
  */
class DeviceManager
{
public:
	/**
	 * @brief Get the singleton instance of DeviceManager.
	 */
	static DeviceManager& instance();

	DeviceManager(const DeviceManager&) = delete;
	DeviceManager& operator=(const DeviceManager&) = delete;

	/**
	 * @brief Enumerate currently connected StreamDock devices.
	 */
	void enumerator();

	/**
	 * @brief Start a background thread to monitor device plug/unplug events.
	 * @param connect_and_run Optional callback to execute when a new device is connected and initialized.
	 */
	void listen(std::function<void(std::shared_ptr<StreamDock>)> connect_and_run = nullptr);

	/**
	 * @brief Stop the device event listener thread.
	 */
	void stopListen();

	/**
	 * @brief Get all currently connected StreamDock devices.
	 * @return A map where the key is the HID path, and the value is the corresponding device object.
	 */
	std::unordered_map<std::string, std::shared_ptr<StreamDock>>& getStreamDocks();

private:
	std::unordered_map<std::string, std::shared_ptr<StreamDock>> streamdocks_; ///< Map of currently connected StreamDock devices. Key is the HID path, value is the associated device object.
	std::thread listener_;                                                     ///< Background thread that listens for device plug/unplug events.
	std::atomic_bool isListening_ = false;                                     ///< Flag indicating whether the device listener is active.
	std::mutex streamdocksMutex_;                                              ///< Mutex to protect access to streamdocks_ and ensure thread safety.
#ifdef _WIN32
	static LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); ///< Windows message handler for receiving device change notifications (e.g., WM_DEVICECHANGE).
	HWND hwnd_ = nullptr;                                                          ///< Handle to the hidden window used to receive Windows device events.
#endif

private:
	DeviceManager() = default;
	~DeviceManager();
};
