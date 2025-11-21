#include "streamdockXL.h"
#include <iostream>

static constexpr auto VID_STREAMDOCK_XL = 0x5548;
static constexpr auto PID_STREAMDOCK_XL = 0x1028;

static constexpr auto VID_STREAMDOCK_XLE = 0x5548;
static constexpr auto PID_STREAMDOCK_XLE = 0x1031;

bool StreamDockXL::registered_XL = []()
{
  StreamDockFactory::instance().registerDevice(
      VID_STREAMDOCK_XL, PID_STREAMDOCK_XL,
      [](const hid_device_info &device_info)
      {
        auto device = std::make_unique<StreamDockXL>(device_info);
        device->init();
        device->initImgHelper();
        return device;
      });
  return true;
}();

bool StreamDockXL::registered_XLE = []()
{
  StreamDockFactory::instance().registerDevice(
      VID_STREAMDOCK_XLE, PID_STREAMDOCK_XLE,
      [](const hid_device_info &device_info)
      {
        auto device = std::make_unique<StreamDockXL>(device_info);
        device->init();
        device->initImgHelper();
        return device;
      });
  return true;
}();

StreamDockXL::StreamDockXL(const hid_device_info &device_info)
    : StreamDock(device_info)
{
  _transport->setReportSize(513, 1025, 0);
  _info->originType = DeviceOriginType::SDXL;
  _info->vendor_id = device_info.vendor_id;
  _info->product_id = device_info.product_id;
  _info->serialNumber = device_info.serial_number;
  std::wcout << "StreamDockXL: " << _info->serialNumber << std::endl;
  _info->width = 1024;
  _info->height = 600;
  _info->keyWidth = 80;
  _info->keyHeight = 80;
  _info->minKey = 1;
  _info->maxKey = 32;
  _info->key_rotate_angle = 180.0f;
  _info->bg_rotate_angle = 180.0f;
  _info->keyEncodeType = ImgType::JPG;
  _feature->isDualDevice = true;
  _feature->hasSecondScreen = false;
  _feature->supportBackGroundGif = true;
  _feature->supportConfig = true;
  _feature->hasRGBLed = true;
  _feature->ledCounts = 6;
  // clang-format off
    _readValueMap = {
        // Normal keys, starting from the bottom-left corner, counted left to right and bottom to top, correspond to keys 1 to 32
        {1,0x19}, {2,0x1A}, {3,0x1B}, {4,0x1C}, {5,0x1D}, {6,0x1E}, {7,0x1F}, {8,0x20},
        {9,0x11}, {10,0x12}, {11,0x13}, {12,0x14}, {13,0x15}, {14,0x16}, {15,0x17}, {16,0x18},
        {17,0x09}, {18,0x0A}, {19,0x0B}, {20,0x0C}, {21,0x0D}, {22,0x0E}, {23,0x0F}, {24,0x10},
        {25,0x01}, {26,0x02}, {27,0x03}, {28,0x04}, {29,0x05}, {30,0x06}, {31,0x07}, {32,0x08},
        /// XL Toggle switch group (from left to right): 33, 35 push up; 34, 36 represent push down
		    {33, 0x21},{34, 0x23},{35,0x24},{36,0x26 },
    };
  // clang-format on
}

RegisterEvent StreamDockXL::dispatchEvent(uint8_t readValue,
                                          uint8_t eventValue)
{
  if ((0x01 <= readValue && readValue <= 0x20) && eventValue == 0x00)
    return RegisterEvent::KeyRelease; /// Normal key release event
  else if ((0x01 <= readValue && readValue <= 0x20) && eventValue == 0x01)
    return RegisterEvent::KeyPress; /// Normal key press event
  else if ((0x21 == readValue || 0x24 == readValue) && eventValue == 0x00)
    return RegisterEvent::ToggleUp;
  else if ((0x23 == readValue || 0x26 == readValue) && eventValue == 0x00)
    return RegisterEvent::ToggleDown;
  return RegisterEvent::EveryThing;
}