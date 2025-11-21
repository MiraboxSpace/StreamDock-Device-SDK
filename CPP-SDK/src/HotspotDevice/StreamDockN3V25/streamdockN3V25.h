#pragma once
#include "DeviceInfo/streamdock.h"
#include "DeviceInfo/streamdockfactory.h"



class StreamDockN3V25 : public StreamDock
{
public:
	explicit StreamDockN3V25(const hid_device_info &device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;

	void changeV2Mode()
	{
		_feature->isDualDevice = false;
		_feature->supportBackGroundGif=false;
	}

private:
	static bool registered_N3V25;
	static bool registered_N3V25E;
};