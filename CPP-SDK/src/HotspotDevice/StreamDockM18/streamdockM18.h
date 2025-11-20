#pragma once
#include "DeviceInfo/streamdock.h"
#include "DeviceInfo/streamdockfactory.h"

class StreamDockM18 : public StreamDock
{
public:
	explicit StreamDockM18(const hid_device_info &device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;
	void changeV2Mode();
private:
	static bool registered_M18;
	static bool registered_M18E;
};