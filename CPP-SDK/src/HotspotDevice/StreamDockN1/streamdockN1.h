#pragma once
#include "DeviceInfo/streamdock.h"
#include "DeviceInfo/streamdockfactory.h"

class StreamDockN1 : public StreamDock
{
public:
	enum class N1MODE
	{
		KEYBOARD_MODE,
		CALCULATOR_MODE,
		SOFTWARE_MODE
	};

	enum class SkinMode
	{
		KEYBOARD = 0x11,
		KEYBOARD_LOCK = 0x1F,
		CALCULATOR = 0xFF,
	};

	enum class SkinStatus
	{
		PRESS = 0,
		RELEASE = 1
	};

public:
	explicit StreamDockN1(const hid_device_info &device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;
	virtual void setBackgroundImgFile(const std::string &filePath, uint32_t timeoutMs = 3000) override;
	virtual void setBackgroundImgStream(const std::string &stream, uint32_t timeoutMs = 3000) override;
	void changeMode(N1MODE mode);
	void changePage(uint8_t page);
	void setSkinBitmap(const std::string &bitmap_path, SkinMode skin_mode, uint8_t skin_page, SkinStatus skin_status, uint8_t key_index, int32_t timeout_ms = 3000);

private:
	static bool registered_N1;
	static bool registered_N1E;
};