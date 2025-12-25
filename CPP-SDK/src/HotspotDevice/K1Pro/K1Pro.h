#pragma once
#include "DeviceInfo/streamdock.h"
#include "DeviceInfo/streamdockfactory.h"

enum class K1ProBackgroundGifPosition : uint8_t {
	KeyScreen = 0x01,
};

class K1Pro : public StreamDock
{
public:
	explicit K1Pro(const hid_device_info& device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;
	void setBackgroundImgFile(const std::string& filePath, uint32_t timeoutMs = 3000) override;

	// Keyboard backlight functions
	void setKeyboardBacklightBrightness(uint8_t brightness);
	void setKeyboardLightingEffects(uint8_t effect);
	void setKeyboardLightingSpeed(uint8_t speed);
	void setKeyboardRgbBacklight(uint8_t red, uint8_t green, uint8_t blue);
	void keyboardOsModeSwitch(uint8_t os_mode);

private:
	static bool registered_K1Pro;
	static bool registered_K1ProEU;
};
