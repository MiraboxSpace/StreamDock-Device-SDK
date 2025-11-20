/**
 * @file iconfiger.h
 * @brief Interface for StreamDock device configuration controllers.
 *
 * Provides an abstract base for sending configuration commands to the device.
 * Implementations can define how config bytes are constructed and transmitted.
 */
#pragma once
#include <streamdock.h>
#include <streamdockconfigcmd.h>


class StreamDock;
class IConfiger
{
public:
	virtual ~IConfiger() = default;

	/**
	 * @brief Send configuration data to the device.
	 * @param configs Configuration bytes constructed according to the device protocol.
	 */
	virtual void setDeviceConfig(std::vector<uint8_t> configs) = 0;
};
