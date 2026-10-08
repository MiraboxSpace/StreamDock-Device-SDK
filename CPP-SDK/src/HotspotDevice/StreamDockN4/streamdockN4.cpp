#include "streamdockN4.h"
#include <iostream>

static constexpr auto VID_STREAMDOCK_N4 = 0x6602;
static constexpr auto PID_STREAMDOCK_N4 = 0x1001;

static constexpr auto VID_STREAMDOCK_N4E = 0x6603;
static constexpr auto PID_STREAMDOCK_N4E = 0x1007;

bool StreamDockN4::registered_N4 = []()
	{
		StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_N4, PID_STREAMDOCK_N4,
			[](const hid_device_info& device_info)
			{
				auto device = std::make_unique<StreamDockN4>(device_info);
				device->init();
				device->initImgHelper();
				return device;
			});
		return true;
	}();

bool StreamDockN4::registered_N4E = []()
	{
		StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_N4E, PID_STREAMDOCK_N4E,
			[](const hid_device_info& device_info)
			{
				auto device = std::make_unique<StreamDockN4>(device_info);
				device->init();
				device->initImgHelper();
				return device;
			});
		return true;
	}();

StreamDockN4::StreamDockN4(const hid_device_info& device_info)
	: StreamDock(device_info)
{
	_transport->setReportSize(513, 1025, 0);
	_info->originType = DeviceOriginType::SDN4;
	_info->vendor_id = device_info.vendor_id;
	_info->product_id = device_info.product_id;
	_info->serialNumber = device_info.serial_number;
	std::wcout << "StreamDockN4: " << _info->serialNumber << std::endl;
	_info->width = 800;
	_info->height = 480;
	_info->keyWidth = 112;
	_info->keyHeight = 112;
	_info->minKey = 5;
	_info->maxKey = 14;
	_info->key_rotate_angle = 180.0f;
	_info->bg_rotate_angle = 180.0f;
	_info->bg_flip_horizonal = true;
	_feature->isDualDevice = true;
	_feature->hasSecondScreen = true;
	_feature->min2rdScreenKey = 1;
	_feature->max2rdScreenKey = 4;
	_feature->_2rdScreenWidth = 176;
	_feature->_2rdScreenHeights = 112;
	_readValueMap = {
		/// LCD keys 1-4 support both press and release events.
		{1, 0x40}, {2, 0x41}, {3, 0x42}, {4, 0x43}, 
		/// Main screen keys use consecutive logical numbers 5-14.
		{5, 0x06},{6, 0x07},{7, 0x08},{8, 0x09}, {9, 0x0A}, {10, 0x01},{11, 0x02},{12, 0x03},{13, 0x04},{14, 0x05},
		/// N4 knob group: 16, 18, 20, 22 represent left rotation; 17, 19, 21, 23 represent right rotation
		{16, 0xA0},{17, 0xA1},{18, 0x50},{19, 0x51},{20, 0x90},{21, 0x91},{22, 0x70},{23, 0x71},
		/// N4 knob group press events: 24 to 27 correspond to knobs 1, 2, 3, and 4 respectively
		{24, 0x37},{25, 0x35},{26, 0x33},{27, 0x36},
		/// N4 secondary screen swipe
		{28, 0x38}, {29, 0x39}
	};
}

uint8_t StreamDockN4::getImageKeyValue(uint8_t keyValue) const
{
	// LCD image slots are 1-4; main screen image slots are 6-15.
	return keyValue >= 5 && keyValue <= 14 ? keyValue + 1 : keyValue;
}

RegisterEvent StreamDockN4::dispatchEvent(uint8_t readValue, uint8_t eventValue)
{
	if ((0x01 <= readValue && readValue <= 0x0A) && eventValue == 0x00)
		return RegisterEvent::KeyRelease; /// Normal key release event
	else if ((0x01 <= readValue && readValue <= 0x0A) && eventValue == 0x01)
		return RegisterEvent::KeyPress; /// Normal key press event
	if ((0x40 <= readValue && readValue <= 0x43) && eventValue == 0x00)
		return RegisterEvent::KeyRelease; /// Secondary screen button release event
	if ((0x40 <= readValue && readValue <= 0x43) && eventValue == 0x01)
		return RegisterEvent::KeyPress; /// Secondary screen button press event
	if ((0xA0 == readValue || 0x50 == readValue || 0x90 == readValue || 0x70 == readValue) && eventValue == 0x00)
		return RegisterEvent::KnobLeft; /// Knob group rotate left event
	if ((0xA1 == readValue || 0x51 == readValue || 0x91 == readValue || 0x71 == readValue) && eventValue == 0x00)
		return RegisterEvent::KnobRight; /// Knob group rotate right event
	if ((0x37 == readValue || 0x35 == readValue || 0x33 == readValue || 0x36 == readValue) && eventValue == 0x01)
		return RegisterEvent::KnobPress; /// Knob group press event
	if ((0x37 == readValue || 0x35 == readValue || 0x33 == readValue || 0x36 == readValue) && eventValue == 0x00)
		return RegisterEvent::KnobRelease; /// Knob group release event
	if ((0x38 == readValue) && eventValue == 0x00)
		return RegisterEvent::SwipeLeft; /// Secondary screen swipe left event
	if ((0x39 == readValue) && eventValue == 0x00)
		return RegisterEvent::SwipeRight; /// Secondary screen swipe right event
	return RegisterEvent::EveryThing;
}
