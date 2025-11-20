#pragma once

#pragma once
#include "streamdock.h"
#include "streamdockfactory.h"

class StreamDock293V2 : public StreamDock
{
public:
	explicit StreamDock293V2(const hid_device_info& device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;

private:
	static bool registered_293V2;
};