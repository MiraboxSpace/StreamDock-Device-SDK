#pragma once

#pragma once
#include "DeviceInfo/streamdock.h"
#include "DeviceInfo/streamdockfactory.h"

class StreamDock293sV3 : public StreamDock
{
public:
	explicit StreamDock293sV3(const hid_device_info& device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;

private:
	static bool registered_293sV3;
};