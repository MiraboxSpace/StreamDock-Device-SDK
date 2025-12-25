#include "K1Pro.h"
#include <iostream>

static constexpr auto VID_K1Pro = 0x6603;
static constexpr auto PID_K1Pro = 0x1015;

static constexpr auto VID_K1ProEU = 0x6603;
static constexpr auto PID_K1ProEU = 0x1019;

bool K1Pro::registered_K1Pro = []()
{
	StreamDockFactory::instance().registerDevice(VID_K1Pro, PID_K1Pro,
												 [](const hid_device_info &device_info)
												 {
													 auto device = std::make_unique<K1Pro>(device_info);
													 device->init();
													 device->initImgHelper();
													 return device;
												 });
	return true;
}();

bool K1Pro::registered_K1ProEU = []()
{
	StreamDockFactory::instance().registerDevice(VID_K1ProEU, PID_K1ProEU,
												 [](const hid_device_info &device_info)
												 {
													 auto device = std::make_unique<K1Pro>(device_info);
													 device->init();
													 device->initImgHelper();
													 return device;
												 });
	return true;
}();

K1Pro::K1Pro(const hid_device_info &device_info)
	: StreamDock(device_info)
{
	_transport->setReportSize(513, 1025, 0);
	_transport->setReportID(0x04);
	_info->originType = DeviceOriginType::K1Pro;
	_info->vendor_id = device_info.vendor_id;
	_info->product_id = device_info.product_id;
	_info->serialNumber = device_info.serial_number;
	std::wcout << L"K1Pro: " << _info->serialNumber << std::endl;
	_info->keyWidth = 64;
	_info->keyHeight = 64;
	_info->minKey = 1;
	_info->maxKey = 6;
	_info->key_rotate_angle = 90.0f;
	_info->keyEncodeType = ImgType::JPG;
	_feature->isDualDevice = true;
	_feature->hasSecondScreen = false;
	_feature->supportBackGroundGif = false;
	_feature->hasRGBLed = false;
	_feature->supportConfig = false;

	// K1Pro key mapping: 6 keys with hardware values 0x05, 0x03, 0x01, 0x06, 0x04, 0x02
	// Normal keys: 1-6 map to hardware codes for image setting
	_readValueMap = {
		/// Normal keys: 1-6 (logical) -> hardware codes for event decoding
		/// KEY_1=0x05, KEY_2=0x03, KEY_3=0x01, KEY_4=0x06, KEY_5=0x04, KEY_6=0x02
		{1, 0x05},
		{2, 0x03},
		{3, 0x01},
		{4, 0x06},
		{5, 0x04},
		{6, 0x02},
		/// Knob press events: 7-9 correspond to knobs 1, 2, 3
		{7, 0x25},
		{8, 0x30},
		{9, 0x31},
		/// Knob rotate left: 10-12 correspond to knobs 1, 2, 3
		{10, 0x50},
		{11, 0x60},
		{12, 0x90},
		/// Knob rotate right: 13-15 correspond to knobs 1, 2, 3
		{13, 0x51},
		{14, 0x61},
		{15, 0x91},
	};
}

RegisterEvent K1Pro::dispatchEvent(uint8_t readValue, uint8_t eventValue)
{
	// Normal key press/release events (hardware codes: 0x05, 0x03, 0x01, 0x06, 0x04, 0x02)
	if ((readValue == 0x05 || readValue == 0x03 || readValue == 0x01 ||
		 readValue == 0x06 || readValue == 0x04 || readValue == 0x02) &&
		eventValue == 0x01)
		return RegisterEvent::KeyPress;
	if ((readValue == 0x05 || readValue == 0x03 || readValue == 0x01 ||
		 readValue == 0x06 || readValue == 0x04 || readValue == 0x02) &&
		eventValue == 0x00)
		return RegisterEvent::KeyRelease;

	// Knob press events (0x25, 0x30, 0x31)
	if ((readValue == 0x25 || readValue == 0x30 || readValue == 0x31) && eventValue == 0x01)
		return RegisterEvent::KnobPress;
	if ((readValue == 0x25 || readValue == 0x30 || readValue == 0x31) && eventValue == 0x00)
		return RegisterEvent::KnobRelease;

	// Knob rotate left events (0x50, 0x60, 0x90)
	if ((readValue == 0x50 || readValue == 0x60 || readValue == 0x90) && eventValue == 0x00)
		return RegisterEvent::KnobLeft;

	// Knob rotate right events (0x51, 0x61, 0x91)
	if ((readValue == 0x51 || readValue == 0x61 || readValue == 0x91) && eventValue == 0x00)
		return RegisterEvent::KnobRight;

	return RegisterEvent::EveryThing;
}

void K1Pro::setBackgroundImgFile(const std::string &filePath, uint32_t timeoutMs)
{
	// K1Pro does not support background image
	(void)filePath;
	(void)timeoutMs;
}
// brightness: Brightness value (0-6)
void K1Pro::setKeyboardBacklightBrightness(uint8_t brightness)
{
	_transport->setKeyboardBacklightBrightness(brightness);
}
// effect: Effect mode identifier (0-9)
// 0 is staic, others are various breathing and wave effects
void K1Pro::setKeyboardLightingEffects(uint8_t effect)
{
	if (effect == 0)
		setKeyboardLightingSpeed(0);
	_transport->setKeyboardLightingEffects(effect);
}
// speed: Speed value for lighting effects (0-7)
void K1Pro::setKeyboardLightingSpeed(uint8_t speed)
{
	_transport->setKeyboardLightingSpeed(speed);
}
// red, green, blue: RGB color values (0-255)
void K1Pro::setKeyboardRgbBacklight(uint8_t red, uint8_t green, uint8_t blue)
{
	_transport->setKeyboardRgbBacklight(red, green, blue);
}
// os_mode: Operating system mode (0 for Windows, 1 for Mac)
void K1Pro::keyboardOsModeSwitch(uint8_t os_mode)
{
	_transport->keyboardOsModeSwitch(os_mode);
}
