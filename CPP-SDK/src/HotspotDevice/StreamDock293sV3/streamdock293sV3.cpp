#include "streamdock293sV3.h"
#include <iostream>

static constexpr auto VID_STREAMDOCK_293sV3 = 0x6603;
static constexpr auto PID_STREAMDOCK_293sV3 = 0x1014;

bool StreamDock293sV3::registered_293sV3 = []()
	{
		StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_293sV3, PID_STREAMDOCK_293sV3,
			[](const hid_device_info& device_info)
			{
				auto device = std::make_unique<StreamDock293sV3>(device_info);
				device->init();
				device->initImgHelper();
				return device;
			});
		return true;
	}();

StreamDock293sV3::StreamDock293sV3(const hid_device_info& device_info)
	: StreamDock(device_info)
{
	_transport->setReportSize(513, 1025, 0);
	_info->originType = DeviceOriginType::SD293s;
	_info->vendor_id = device_info.vendor_id;
	_info->product_id = device_info.product_id;
	_info->serialNumber = device_info.serial_number;
	std::wcout << "StreamDock293sV3: " << _info->serialNumber << std::endl;
	_info->width = 854;
	_info->height = 480;
	_info->keyWidth = 96;
	_info->keyHeight = 96;
	_info->minKey = 1;
	_info->maxKey = 15;
	_info->key_rotate_angle = 270.0f;
	_info->bg_rotate_angle = 270.0f;
	_feature->isDualDevice = true;
	_feature->hasSecondScreen = true;
	_feature->min2rdScreenKey = 16;
	_feature->max2rdScreenKey = 18;
	_feature->_2rdScreenWidth = 80;
	_feature->_2rdScreenHeights = 80;
	_readValueMap = {
		/// Normal keys, starting from the top-right corner, counted top to bottom and right to left, correspond to keys 1 to 15
		/// The secondary screen of the 293sV3 has no press or release actions; it only supports display functionality
		{1, 0x01}, {2, 0x02}, {3, 0x03}, {4, 0x04}, {5, 0x05},
		{6, 0x06}, {7, 0x07}, {8, 0x08}, {9, 0x09}, {10, 0x0A},
		{11, 0x0B},{12, 0x0C},{13, 0x0D},{14, 0x0E},{15, 0x0F},
	};
}

RegisterEvent StreamDock293sV3::dispatchEvent(uint8_t readValue, uint8_t eventValue)
{
	if ((0x01 <= readValue && readValue <= 0x0F) && eventValue == 0x00)
		return RegisterEvent::KeyRelease; /// Normal key release event
	else if ((0x01 <= readValue && readValue <= 0x0F) && eventValue == 0x01)
		return RegisterEvent::KeyPress; /// Normal key press event
	return RegisterEvent::EveryThing;
}
