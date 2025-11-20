#pragma once
#include "DeviceInfo/streamdock.h"
#include "DeviceInfo/streamdockfactory.h"


class StreamDockM3 : public StreamDock
{
public:
	explicit StreamDockM3(const hid_device_info& device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;

private:
	static bool registered_M3;
	// static bool registered_M3E;
};

enum class M3ConfigEnumerate : uint8_t {
	LedFollowKeyLight,		///< LED follows key light
	KeyLightOnDisconne,		///< Key light when disconnectedct,
	CheckUsbPower,			///< Check USB power supply
	EnableVibration,		///< enable global vibration
	ResetUsbReport,			///< Reset USB reporting
	EnableBootVideo,		///< Enable boot video
	ConfigCount				// 用于数组大小
};

template <>
struct ConfigTraits<M3ConfigEnumerate> {
	static constexpr size_t count = static_cast<size_t>(M3ConfigEnumerate::ConfigCount);
};
