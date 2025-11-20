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

public:
	explicit StreamDockN1(const hid_device_info& device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;
	virtual void setBackgroundImgFile(const std::string& filePath, uint32_t timeoutMs = 3000) override {
	}
	virtual void setBackgroundImgStream(const std::string& stream, uint32_t timeoutMs = 3000) override {
	}
	void changeMode(N1MODE mode);

private:
	static bool registered_N1;
	static bool registered_N1E;
};