#pragma once
#include "DeviceInfo/streamdock.h"
#include "DeviceInfo/streamdockfactory.h"

class StreamDockN4 : public StreamDock
{
public:
	explicit StreamDockN4(const hid_device_info& device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;

private:
	static bool registered_N4;
	static bool registered_N4E;
};