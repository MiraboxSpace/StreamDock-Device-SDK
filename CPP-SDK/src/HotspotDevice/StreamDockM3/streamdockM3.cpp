#include "streamdockM3.h"
#include <iostream>

static constexpr auto VID_STREAMDOCK_M3 = 0x5548;
static constexpr auto PID_STREAMDOCK_M3 = 0x1020;

// static constexpr auto VID_STREAMDOCK_M3E = 0x5548;
// static constexpr auto PID_STREAMDOCK_M3E = ;

bool StreamDockM3::registered_M3 = []()
{
    StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_M3, PID_STREAMDOCK_M3,
                                                 [](const hid_device_info &device_info)
                                                 {
                                                     auto device = std::make_unique<StreamDockM3>(device_info);
                                                     device->init();
                                                     device->initImgHelper();
                                                     return device;
                                                 });
    return true;
}();

// bool StreamDockM3::registered_M3E = []()
// {
//     StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_M3E, PID_STREAMDOCK_M3E,
//                                                  [](const hid_device_info &device_info)
//                                                  {
//                                                      auto device = std::make_unique<StreamDockM3>(device_info);
//                                                      device->init();
//                                                      device->initImgHelper();
//                                                      return device;
//                                                  });
//     return true;
// }();

StreamDockM3::StreamDockM3(const hid_device_info &device_info)
    : StreamDock(device_info)
{
    _transport->setReportSize(513, 1025, 0);
    _info->originType = DeviceOriginType::SDM3;
    _info->vendor_id = device_info.vendor_id;
    _info->product_id = device_info.product_id;
    _info->serialNumber = device_info.serial_number;
    std::wcout << "StreamDockM3: " << _info->serialNumber << std::endl;
    _info->width = 854;
    _info->height = 480;
    _info->keyWidth = 96;
    _info->keyHeight = 96;
    _info->minKey = 1;
    _info->maxKey = 15;
    _info->key_rotate_angle = -90.0f;
    _info->bg_rotate_angle = -90.0f;
    _info->keyEncodeType = ImgType::PNG;
    _feature->isDualDevice = true;
    _feature->hasSecondScreen = false;
    _feature->supportBackGroundGif = true;
    _feature->hasRGBLed = false;
    _feature->supportConfig = false;
    _feature->ledCounts = 0;
    // clang-format off
	_readValueMap = {
		/// Normal keys, starting from the bottom-left corner, counted left to right and bottom to top, correspond to keys 1 to 15
		{1, 0x0b},{2, 0x0c},{3, 0x0d},{4, 0x0e},{5, 0x0f},
        {6, 0x06},{7, 0x07},{8, 0x08},{9, 0x09},{10, 0x0a},
        {11, 0x01},{12, 0x02},{13, 0x03},{14, 0x04},{15, 0x05},
		/// M3 knob group (down to top): 16, 18, 20 represent left rotation; 17, 19, 21 represent right rotation
		{16, 0xA0},{17, 0xA1},{18, 0x90},{19, 0x91},{20, 0x50},{21, 0x51},
		/// M3 knob group press events (down to top): 22 to 24 correspond to knobs 1, 2, 3 respectively
		{22, 0x37},{23, 0x33},{24, 0x35}
	};
    // clang-format on
}

RegisterEvent StreamDockM3::dispatchEvent(uint8_t readValue, uint8_t eventValue)
{
    if ((0x01 <= readValue && readValue <= 0x0f) && eventValue == 0x00)
        return RegisterEvent::KeyRelease; /// Normal key release event
    else if ((0x01 <= readValue && readValue <= 0x0f) && eventValue == 0x01)
        return RegisterEvent::KeyPress; /// Normal key press event
    if ((0x40 <= readValue && readValue <= 0x43) && eventValue == 0x00)
        return RegisterEvent::KeyRelease; /// Secondary screen button release event
    if ((0xA0 == readValue || 0x90 == readValue || 0x50 == readValue) && eventValue == 0x00)
        return RegisterEvent::KnobLeft; /// Knob group rotate left event
    if ((0xA1 == readValue || 0x91 == readValue || 0x51 == readValue) && eventValue == 0x00)
        return RegisterEvent::KnobRight; /// Knob group rotate right event
    if ((0x37 == readValue || 0x33 == readValue || 0x35 == readValue) && eventValue == 0x01)
        return RegisterEvent::KnobPress; /// Knob group press event
    // if ((0x37 == readValue || 0x33 == readValue || 0x35 == readValue) && eventValue == 0x00)
    //     return RegisterEvent::KnobRelease; /// Knob group release event
    return RegisterEvent::EveryThing;
}