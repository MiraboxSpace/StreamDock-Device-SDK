#include "streamdock293V2.h"
#include <iostream>

static constexpr auto VID_STREAMDOCK_293V2 = 0x5500;
static constexpr auto PID_STREAMDOCK_293V2 = 0x1001;

bool StreamDock293V2::registered_293V2 = []()
	{
		StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_293V2, PID_STREAMDOCK_293V2,
			[](const hid_device_info& device_info)
			{
				auto device = std::make_unique<StreamDock293V2>(device_info);
				device->init();
				device->initImgHelper();
				return device;
			});
		return true;
	}();
StreamDock293V2::StreamDock293V2(const hid_device_info& device_info)
	: StreamDock(device_info)
{
	_transport->setReportSize(513, 513, 0);
	_info->originType = DeviceOriginType::SD293;
	_info->vendor_id = device_info.vendor_id;
	_info->product_id = device_info.product_id;
	_info->serialNumber = device_info.serial_number;
	std::wcout << "StreamDock293V2: " << _info->serialNumber << std::endl;
	_info->width = 800;
	_info->height = 480;
	_info->keyWidth = 100;
	_info->keyHeight = 100;
	_info->minKey = 1;
	_info->maxKey = 15;
	_info->key_rotate_angle = 180.0f;
	_info->bg_rotate_angle = 180.0f;
	_info->backgroundEncodeType = ImgType::RAW;
	_info->backgroundEncodeFormat = ImgFormat::BGR888;
	_readValueMap = {
		/// Normal keys, starting from the bottom-left corner, counted left to right and bottom to top, correspond to keys 1 to 15
		{1, 0x0B},{2, 0x0C},{3, 0x0D},{4, 0x0E},{5, 0x0F},
		{6, 0x06},{7, 0x07},{8, 0x08},{9, 0x09},{10, 0x0A},
		{11, 0x01},{12, 0x02},{13, 0x03},{14, 0x04},{15, 0x05},
	};
}

RegisterEvent StreamDock293V2::dispatchEvent(uint8_t readValue, uint8_t eventValue)
{
	if ((0x01 <= readValue && readValue <= 0x0F) && eventValue == 0x00)
		return RegisterEvent::KeyRelease; ///  Normal key release event
	else if ((0x01 <= readValue && readValue <= 0x0F) && eventValue == 0x01)
		return RegisterEvent::KeyPress; /// Normal key press event
	return RegisterEvent::EveryThing;
}
