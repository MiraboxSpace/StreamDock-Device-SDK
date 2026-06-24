#include "streamdockMini.h"
#include <iostream>

static constexpr auto VID_STREAMDOCK_MINI = 0x5548;
static constexpr auto PID_STREAMDOCK_MINI = 0x1036;

static constexpr auto VID_STREAMDOCK_MINIW = 0x5548;
static constexpr auto PID_STREAMDOCK_MINIW = 0x1037;

bool StreamDockMini::registered_Mini = []()
{
	StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_MINI, PID_STREAMDOCK_MINI,
												 [](const hid_device_info& device_info)
												 {
													 auto device = std::make_unique<StreamDockMini>(device_info);
													 device->init();
													 device->initImgHelper();
													 return device;
												 });
	return true;
}();

bool StreamDockMini::registered_MiniW = []()
{
	StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_MINIW, PID_STREAMDOCK_MINIW,
												 [](const hid_device_info& device_info)
												 {
													 auto device = std::make_unique<StreamDockMini>(device_info);
													 device->init();
													 device->initImgHelper();
													 return device;
												 });
	return true;
}();

StreamDockMini::StreamDockMini(const hid_device_info& device_info)
	: StreamDock(device_info)
{
	_transport->setReportSize(513, 1025, 0);
	_info->originType = DeviceOriginType::SDMini;
	_info->vendor_id = device_info.vendor_id;
	_info->product_id = device_info.product_id;
	_info->serialNumber = device_info.serial_number;
	std::wcout << "StreamDockMini: " << _info->serialNumber << std::endl;
	_info->width = 320;
	_info->height = 240;
	_info->keyWidth = 64;
	_info->keyHeight = 64;
	_info->minKey = 1;
	_info->maxKey = 6;
	_info->key_rotate_angle = -90.0f;
	_info->bg_rotate_angle = -90.0f;
	_feature->isDualDevice = true;
	_feature->hasSecondScreen = false;
	_feature->supportBackGroundGif = false;
	_feature->supportKeyJpegPngStream = false;
	_feature->hasRGBLed = true;
	_feature->supportConfig = false;
	_feature->ledCounts = 12;
	// clang-format off
	_readValueMap = {
		/// Normal keys, starting from the top-left corner, counted left to right and top to bottom, correspond to keys 1 to 6
		{1, 0x01}, {2, 0x02}, {3, 0x03}, {4, 0x04}, {5, 0x05}, {6, 0x06},
		/// Mini DIP switch 1: 7 left, 8 right, 9 press
		{7, 0x24}, {8, 0x26}, {9, 0x25},
		/// Mini DIP switch 2: 10 left, 11 right, 12 press
		{10, 0x21}, {11, 0x23}, {12, 0x22},
	};
	// clang-format on
}

RegisterEvent StreamDockMini::dispatchEvent(uint8_t readValue, uint8_t eventValue)
{
	if ((0x01 <= readValue && readValue <= 0x06) && eventValue == 0x00)
		return RegisterEvent::KeyRelease;
	if ((0x01 <= readValue && readValue <= 0x06) && eventValue == 0x01)
		return RegisterEvent::KeyPress;
	if ((readValue == 0x24 || readValue == 0x21) && eventValue == 0x01)
		return RegisterEvent::DIPLeft;
	if ((readValue == 0x24 || readValue == 0x21) && eventValue == 0x00)
		return RegisterEvent::DIPLeftEnd;
	if ((readValue == 0x26 || readValue == 0x23) && eventValue == 0x01)
		return RegisterEvent::DIPRight;
	if ((readValue == 0x26 || readValue == 0x23) && eventValue == 0x00)
		return RegisterEvent::DIPRightEnd;
	if ((readValue == 0x25 || readValue == 0x22) && eventValue == 0x01)
		return RegisterEvent::DIPPress;
	if ((readValue == 0x25 || readValue == 0x22) && eventValue == 0x00)
		return RegisterEvent::DIPRelease;
	return RegisterEvent::EveryThing;
}
