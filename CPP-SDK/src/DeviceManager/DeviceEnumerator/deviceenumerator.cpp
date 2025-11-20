#include "deviceenumerator.h"
#include <iostream>
#include <iomanip>
#include <toolkit.h>

DeviceEnumerator::DeviceInfo::DeviceInfo(const hid_device_info& info)
{
	_path = info.path ? info.path : "";
	_vendor_id = info.vendor_id;
	_product_id = info.product_id;
	_release_number = info.release_number;
	_usage_page = info.usage_page;
	_usage = info.usage;
	_interface_number = info.interface_number;

	if (info.serial_number)
		_serial_number = info.serial_number;
	if (info.manufacturer_string)
		_manufacturer_string = info.manufacturer_string;
	if (info.product_string)
		_product_string = info.product_string;
}

hid_device_info DeviceEnumerator::DeviceInfo::toPureHidDeviceInfo()
{
	hid_device_info info{};
	info.path = _path.data();
	info.vendor_id = _vendor_id;
	info.product_id = _product_id;
	info.serial_number = _serial_number.data();
	info.release_number = _release_number;
	info.manufacturer_string = _manufacturer_string.data();
	info.product_string = _product_string.data();
	info.usage = _usage;
	info.usage_page = _usage_page;
	info.interface_number = _interface_number;
	return info;
}

DeviceEnumerator::DeviceEnumerator()
{
	hid_init();
}
DeviceEnumerator::~DeviceEnumerator()
{
	hid_exit();
}

void DeviceEnumerator::enumerate(bool log_out)
{
	_deviceList.clear();
	_pathMap.clear();
	_vidpidMap.clear();

	struct hid_device_info* devs = hid_enumerate(0x0, 0x0);
	struct hid_device_info* cur = devs;

	while (cur)
	{
		auto dev = std::make_shared<DeviceInfo>(*cur);

		_deviceList.push_back(dev);
		_pathMap[dev->_path] = dev;

		uint32_t key = (static_cast<uint32_t>(dev->_vendor_id) << 16) | dev->_product_id;
		_vidpidMap.emplace(key, std::weak_ptr<DeviceInfo>(dev));

		cur = cur->next;
	}

	if (log_out)
	{
		int index = 1;
		for (const auto& dev : _deviceList)
		{
			ToolKit::print("\nDevice #", index++, ":");
			ToolKit::print("  Path:", dev->_path);
			std::ostringstream oss;
			oss << "  VID:PID: 0x"
				<< std::hex << std::setw(4) << std::setfill('0') << dev->_vendor_id
				<< ":0x" << std::setw(4) << dev->_product_id;
			ToolKit::print(oss.str());
			ToolKit::wprint(L"  Vendor:", dev->_manufacturer_string);
			ToolKit::wprint(L"  Product:", dev->_product_string);
			ToolKit::wprint(L"  Serial Number:", dev->_serial_number);
			ToolKit::print("  Release Number:", L"0x", dev->_release_number);
			ToolKit::print("  Interface Number:", dev->_interface_number);
			ToolKit::print("  Usage Page:", L"0x", dev->_usage_page);
			ToolKit::print("  Usage:", L"0x", dev->_usage);
		}
	}
	hid_free_enumeration(devs);
}

const std::vector<std::shared_ptr<DeviceEnumerator::DeviceInfo>>& DeviceEnumerator::currDevices() const
{
	return _deviceList;
}

DeviceEnumerator& DeviceEnumerator::instance()
{
	static DeviceEnumerator enumerator;
	return enumerator;
}

std::shared_ptr<DeviceEnumerator::DeviceInfo> DeviceEnumerator::find_by_path(const std::string& path) const
{
	auto it = _pathMap.find(path);
	return (it != _pathMap.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<DeviceEnumerator::DeviceInfo>> DeviceEnumerator::find_by_vidpid(unsigned short vid, unsigned short pid) const
{
	std::vector<std::shared_ptr<DeviceInfo>> results;
	uint32_t key = (static_cast<uint32_t>(vid) << 16) | pid;

	auto range = _vidpidMap.equal_range(key);
	for (auto it = range.first; it != range.second; ++it)
	{
		if (auto dev = it->second.lock())
		{
			results.push_back(dev);
		}
	}

	return results;
}

hid_device* DeviceEnumerator::open_device(const std::string& path) const
{
	auto dev = find_by_path(path);
	dev->_dev_ptr = hid_open_path(dev->_path.data());
	return dev->_dev_ptr;
}

void DeviceEnumerator::close_device(const std::string& path) const
{
	auto dev = find_by_path(path);
	hid_close(dev->_dev_ptr);
}

bool DeviceEnumerator::send_to_path(const std::string& path, const std::vector<uint8_t>& data) const
{
	auto dev = find_by_path(path);
	if (!dev || data.empty())
		return false;
	if (!dev->_dev_ptr && !open_device(path))
		return false;
	int res = hid_write(dev->_dev_ptr, data.data(), data.size());
	return res >= 0;
}