#include "streamdockN3V2.h"
#include <iostream>

static constexpr auto VID_STREAMDOCK_N3V2 = 0x6602;
static constexpr auto PID_STREAMDOCK_N3V2 = 0x1000;

static constexpr auto VID_STREAMDOCK_N3V2E = 0x6602;
static constexpr auto PID_STREAMDOCK_N3V2E = 0x1002;

bool StreamDockN3::registered_N3V2 = []()
	{
		StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_N3V2, PID_STREAMDOCK_N3V2,
			[](const hid_device_info& device_info)
			{
				auto device = std::make_unique<StreamDockN3>(device_info);
				device->init();
				device->initImgHelper();
				return device;
			});
		return true;
	}();

bool StreamDockN3::registered_N3V2E = []()
	{
		StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_N3V2E, PID_STREAMDOCK_N3V2E,
			[](const hid_device_info& device_info)
			{
				auto device = std::make_unique<StreamDockN3>(device_info);
				device->init();
				device->initImgHelper();
				return device;
			});
		return true;
	}();

StreamDockN3::StreamDockN3(const hid_device_info& device_info)
	: StreamDock(device_info)
{
	_transport->setReportSize(513, 1025, 0);
	_info->originType = DeviceOriginType::SDN3;
	_info->vendor_id = device_info.vendor_id;
	_info->product_id = device_info.product_id;
	_info->serialNumber = device_info.serial_number;
	std::wcout << "StreamDockN3: " << _info->serialNumber << std::endl;
	_info->width = 320;
	_info->height = 240;
	_info->keyWidth = 64;
	_info->keyHeight = 64;
	_info->backgroundEncodeType = ImgType::RAW;
	_info->backgroundEncodeFormat = ImgFormat::RGB16;
	_info->minKey = 1;
	_info->maxKey = 6;
	_info->key_rotate_angle = 0.0f;
	_info->bg_rotate_angle = 90.0f;
	_readValueMap = {
		/// Normal keys, starting from the top-left corner, counted left to right and top to bottom, correspond to keys 1 to 6
		{1, 0x01}, {2, 0x02}, {3, 0x03}, {4, 0x04},{5, 0x05},{6, 0x06},
		/// Three black buttons at the bottom, from left to right: 25, 30, 31
		{7, 0x25},{8, 0x30},{9, 0x31},
		/// Three knob press buttons on the left side: bottom-left is 33, bottom-right is 34, top is 35
		{10, 0x33},{11, 0x34},{12, 0x35},
		/// Three knob left rotations on the left side: bottom-left is 90, bottom-right is 60, top is 50
		{13, 0x90},{14, 0x60},{15, 0x50},
		/// Three knob right rotations on the left side: bottom-left is 91, bottom-right is 61, top is 51
		{16, 0x91},{17, 0x61},{18, 0x51},
	};
}

RegisterEvent StreamDockN3::dispatchEvent(uint8_t readValue, uint8_t eventValue)
{
	if ((0x01 <= readValue && readValue <= 0x06) && eventValue == 0x00)
		return RegisterEvent::KeyRelease; /// Normal key release event
	if ((0x01 <= readValue && readValue <= 0x06) && eventValue == 0x01)
		return RegisterEvent::KeyPress; /// Normal key press event
	if ((0x25 <= readValue && readValue <= 0x31) && eventValue == 0x00)
		return RegisterEvent::KeyRelease; /// Bottom button release event
	if ((0x25 <= readValue && readValue <= 0x31) && eventValue == 0x01)
		return RegisterEvent::KeyPress; /// Bottom button press event
	if ((0x33 <= readValue && readValue <= 0x35) && eventValue == 0x00)
		return RegisterEvent::KnobPress; /// Knob press event
	if ((0x90 == readValue || readValue == 0x60 || readValue == 0x50) && eventValue == 0x00)
		return RegisterEvent::KnobLeft; /// Knob rotate left event
	if ((0x91 == readValue || readValue == 0x61 || readValue == 0x51) && eventValue == 0x00)
		return RegisterEvent::KnobRight; /// Knob rotate right event
	return RegisterEvent::EveryThing;
}
