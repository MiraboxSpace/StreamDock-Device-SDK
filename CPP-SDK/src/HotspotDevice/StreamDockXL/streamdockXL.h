#pragma once
#include "DeviceInfo/streamdock.h"
#include "DeviceInfo/streamdockfactory.h"

class StreamDockXL : public StreamDock
{
public:
    explicit StreamDockXL(const hid_device_info &device_info);
    virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;

private:
    static bool registered_XL;
    static bool registered_XLE;
};

enum class XLConfigEnumerate : uint8_t
{
    LedFollowKeyLight, ///< LED follows key light
    ConfigCount        // Used for array size
};

template <>
struct ConfigTraits<XLConfigEnumerate>
{
    static constexpr size_t count = static_cast<size_t>(XLConfigEnumerate::ConfigCount);
};
