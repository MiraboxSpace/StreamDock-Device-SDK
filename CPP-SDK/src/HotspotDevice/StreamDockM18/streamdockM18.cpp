#include "streamdockM18.h"
#include <iostream>

static constexpr auto VID_STREAMDOCK_M18 = 0x6603;
static constexpr auto PID_STREAMDOCK_M18 = 0x1009;

static constexpr auto VID_STREAMDOCK_M18E = 0x6603;
static constexpr auto PID_STREAMDOCK_M18E = 0x1012;

bool StreamDockM18::registered_M18 = []()
	{
		StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_M18, PID_STREAMDOCK_M18,
			[](const hid_device_info& device_info)
			{
				auto device = std::make_unique<StreamDockM18>(device_info);
				device->init();
				device->initImgHelper();
				return device;
			});
		return true;
	}();

bool StreamDockM18::registered_M18E = []()
	{
		StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_M18E, PID_STREAMDOCK_M18E,
			[](const hid_device_info& device_info)
			{
				auto device = std::make_unique<StreamDockM18>(device_info);
				device->init();
				device->initImgHelper();
				return device;
			});
		return true;
	}();

StreamDockM18::StreamDockM18(const hid_device_info& device_info)
	: StreamDock(device_info)
{
	_transport->setReportSize(513, 1025, 0);
	_info->originType = DeviceOriginType::SDM18;
	_info->vendor_id = device_info.vendor_id;
	_info->product_id = device_info.product_id;
	_info->serialNumber = device_info.serial_number;
	// std::wcout << "StreamDockM18: " << _info->serialNumber << std::endl;
	_info->width = 480;
	_info->height = 272;
	_info->keyWidth = 64;
	_info->keyHeight = 64;
	_info->minKey = 1;
	_info->maxKey = 15;
	_info->key_rotate_angle = 0.0f;
	_info->bg_rotate_angle = 0.0f;
	_feature->isDualDevice = true;
	_feature->hasRGBLed = true;
	_feature->ledCounts = 24;
	_readValueMap = {
		/// Normal keys, starting from the bottom-left corner, counted left to right and bottom to top, correspond to keys 1 to 15
		{1, 0x0B}, {2, 0x0C}, {3, 0x0D}, {4, 0x0E}, {5, 0x0F},
		{6, 0x06}, {7, 0x07}, {8, 0x08}, {9, 0x09}, {10,0x0A},
		{11,0x01}, {12,0x02}, {13,0x03}, {14,0x04}, {15,0x05},
		/// The following three buttons, from left to right, are 25, 30, and 31
		{16,0x25}, {17,0x30}, {18,0x31}
	};
}

void StreamDockM18::changeV2Mode(){
	_feature->isDualDevice = false;
	_feature->supportBackGroundGif=false;
}

RegisterEvent StreamDockM18::dispatchEvent(uint8_t readValue, uint8_t eventValue)
{
	if ((0x01 <= readValue && readValue <= 0x0F) && eventValue == 0x00)
		return RegisterEvent::KeyRelease; /// Normal key release event
	else if ((0x01 <= readValue && readValue <= 0x0F) && eventValue == 0x01)
		return RegisterEvent::KeyPress; /// Normal key press event
	if ((0x25 <= readValue && readValue <= 0x31) && eventValue == 0x00)
		return RegisterEvent::KeyRelease; /// Bottom button release event
	else if ((0x25 <= readValue && readValue <= 0x31) && eventValue == 0x01)
		return RegisterEvent::KeyPress; /// Bottom button press event
	return RegisterEvent::EveryThing;
}
