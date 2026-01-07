#pragma once
#include "DeviceInfo/streamdock.h"
#include "DeviceInfo/streamdockfactory.h"

enum class N4ProBackgroundGifPostion : uint8_t {
	KeyScreen = 0x01,
	TouchScreen = 0x02,
	All = 0x03,
};

class StreamDockN4Pro : public StreamDock
{
public:
public:
	explicit StreamDockN4Pro(const hid_device_info& device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;
	void registerTouchBarCallback(std::function<void(const std::vector<uint8_t>)> callback, bool asyncRun);

private:
	static bool registered_N4Pro;
	static bool registered_N4ProE;
};

enum class N4ProConfigEnumerate : uint8_t {
	LedFollowKeyLight,		///< LED follows key light
	KeyLightOnDisconne,		///< Key light when disconnectedct,
	CheckUsbPower,			///< Check USB power supply
	EnableVibration,		///< enable global vibration
	ResetUsbReport,			///< Reset USB reporting
	EnableBootVideo,		///< Enable boot video
	ConfigCount				// Used for array size
};

template <>
struct ConfigTraits<N4ProConfigEnumerate> {
	static constexpr size_t count = static_cast<size_t>(N4ProConfigEnumerate::ConfigCount);
};
