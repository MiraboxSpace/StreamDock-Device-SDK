/**
 * @file DeviceEnumerator.h
 * @brief Utility class for HID device enumeration and operations.
 *
 * Overview:
 * - Enumerate currently connected HID devices
 * - Find devices by HID path or VID/PID
 * - Open and close device handles
 * - Send raw data to a device
 *
 * Provides a cross-platform wrapper based on hidapi.
 */
#pragma once
#include <memory>
#include <vector>
#include <hidapi.h>
#include <unordered_map>
#include <string>

 /**
 * @class DeviceEnumerator
 * @brief Singleton HID device enumerator for managing device info and communication.
 *
 * Wraps hidapi functionality to enumerate and manage HID devices.
 * Supports searching by path or VID/PID, and provides methods to open,
 * close, and send data to devices.
 *
 * Internally maintains a list of current devices and their mappings
 * to avoid repeated enumeration.
 */
class DeviceEnumerator
{
public:
	/**
	 * @struct DeviceInfo
	 * @brief Represents metadata for a single HID device.
	 *
	 * Stores raw information from hidapi enumeration and can convert back to
	 * hid_device_info format. Includes fields such as device path, VID, PID,
	 * serial number, manufacturer name, and product string.
	 */
	struct DeviceInfo
	{
		DeviceInfo(const hid_device_info& info);
		hid_device_info toPureHidDeviceInfo();
		std::string _path;
		std::wstring _serial_number;
		std::wstring _manufacturer_string;
		std::wstring _product_string;
		unsigned short _vendor_id = 0;
		unsigned short _product_id = 0;
		unsigned short _release_number = 0;
		unsigned short _usage_page = 0;
		unsigned short _usage = 0;
		int _interface_number = -1;
		hid_device* _dev_ptr = nullptr;
	};

public:
	DeviceEnumerator(const DeviceEnumerator&) = delete;
	DeviceEnumerator& operator=(const DeviceEnumerator&) = delete;
	DeviceEnumerator(DeviceEnumerator&&) = default;
	DeviceEnumerator& operator=(DeviceEnumerator&&) = default;

	/**
	 * @brief Get the singleton instance.
	 */
	static DeviceEnumerator& instance();

public:
	/**
	 * @brief Enumerate all connected HID devices.
	 * @param log_out Whether to print debug output.
	 */
	void enumerate(bool log_out = false);

	/**
	 * @brief Get the current list of enumerated devices.
	 */
	const std::vector<std::shared_ptr<DeviceInfo>>& currDevices() const;

	/**
	 * @brief Find a device by its HID path.
	 * @param path The device's HID path.
	 * @return A pointer to the matching DeviceInfo, or nullptr if not found.
	 */
	std::shared_ptr<DeviceInfo> find_by_path(const std::string& path) const;

	/**
	 * @brief Find all devices matching a given VID and PID.
	 * @param vid Vendor ID.
	 * @param pid Product ID.
	 * @return A list of matching devices.
	 */
	std::vector<std::shared_ptr<DeviceInfo>> find_by_vidpid(unsigned short vid, unsigned short pid) const;

	/**
	 * @brief Open a device by its path.
	 * @param path HID path to the device.
	 * @return A pointer to the opened hid_device, or nullptr on failure.
	 */
	hid_device* open_device(const std::string& path) const;

	/**
	 * @brief Close the device associated with the given path.
	 * @param path HID path to the device.
	 */
	void close_device(const std::string& path) const;

	/**
	 * @brief Send raw data to the device with the given path.
	 * @param path HID path of the target device.
	 * @param data The data to send.
	 * @return True on success, false otherwise.
	 */
	bool send_to_path(const std::string& path, const std::vector<uint8_t>& data) const;

private:
	DeviceEnumerator();
	~DeviceEnumerator();

private:
	hid_device_info* _info_list = nullptr;                                    ///< Raw device info list returned by hidapi.
	std::vector<std::shared_ptr<DeviceInfo>> _deviceList;                     ///< List of currently enumerated devices.
	std::unordered_map<std::string, std::shared_ptr<DeviceInfo>> _pathMap;    ///< Mapping from HID path to device info.
	std::unordered_multimap<uint32_t, std::weak_ptr<DeviceInfo>> _vidpidMap;  ///< Mapping from VID/PID (as a combined key) to device info (non-owning).

};