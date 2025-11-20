/**
 * @file streamdockfactory.h
 * @brief Factory class for creating StreamDock device instances based on VID/PID.
 *
 * Overview:
 * - Allows registering specific device types (VID/PID) with their creation logic
 * - Supports dynamic creation of StreamDock subclasses based on connected devices
 * - Centralized management of supported device mappings
 *
 * Usage Example:
 *
 * 1. Define a device subclass, e.g., StreamDockM18:
 *
 *     class StreamDockM18 : public StreamDock {
 *     public:
 *         explicit StreamDockM18(const hid_device_info& device_info);
 *         RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;
 *     private:
 *         static bool registered_M18;
 *     };
 *
 * 2. Register it in its implementation file:
 *
 *     static constexpr auto VID_STREAMDOCK_M18 = 0x6603;
 *     static constexpr auto PID_STREAMDOCK_M18 = 0x1009;
 *
 *     bool StreamDockM18::registered_M18 = []() {
 *         StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_M18, PID_STREAMDOCK_M18,
 *             [](const hid_device_info& info) {
 *                 auto device = std::make_unique<StreamDockM18>(info);
 *                 device->init();
 *                 device->initImgHelper();
 *                 return device;
 *             });
 *         return true;
 *     }();
 *
 * 3. Create device instance dynamically:
 *
 *     const hid_device_info& info = ...;
 *     auto device = StreamDockFactory::instance().create(info.vendor_id, info.product_id, info);
 *     if (device) {
 *         device->start();
 *     }
 */
#pragma once
#include "streamdock.h"
#include <functional>
#include <memory>
#include <map>

/**
 * @class StreamDockFactory
 * @brief Factory class for creating StreamDock instances based on VID/PID.
 *
 * Devices are registered using registerDevice(), and instances are created
 * using create() based on the vendor ID and product ID.
 *
 * Implements the singleton pattern. Copy and move operations are disabled.
 */
class StreamDockFactory
{
public:
	/**
	 * @brief Alias for the creator function used to construct StreamDock-derived devices.
	 */
	using Creator = std::function<std::unique_ptr<StreamDock>(const hid_device_info& device_info)>;

	/**
	 * @brief Get the singleton instance of the factory.
	 */
	static StreamDockFactory& instance();

	StreamDockFactory(const StreamDockFactory&) = delete;
	StreamDockFactory& operator=(const StreamDockFactory&) = delete;
	StreamDockFactory(StreamDockFactory&&) = delete;
	StreamDockFactory& operator=(StreamDockFactory&&) = delete;

	/**
	 * @brief Register a constructor function for a specific VID/PID.
	 * @param vid Vendor ID.
	 * @param pid Product ID.
	 * @param creator Function that creates the corresponding device instance.
	 */
	void registerDevice(uint16_t vid, uint16_t pid, Creator creator);

	/**
	 * @brief Check if a device with the given VID/PID is registered.
	 * @return True if registered, false otherwise.
	 */
	bool exist(uint16_t vid, uint16_t pid) const;

	/**
	 * @brief Create a StreamDock instance based on the VID/PID and device info.
	 * @param device_info Device descriptor from hidapi.
	 * @return The constructed device instance, or nullptr if not registered.
	 */
	std::unique_ptr<StreamDock> create(uint16_t vid, uint16_t pid, const hid_device_info& device_info);

private:
	StreamDockFactory() = default;
	std::map<std::pair<uint16_t, uint16_t>, Creator> deviceMap;  ///< Mapping of <VID, PID> pairs to device creator functions.
};
