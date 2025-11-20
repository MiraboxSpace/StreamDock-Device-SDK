/**
 * @file configer.h
 * @brief Concrete configuration controller for StreamDock devices.
 *
 * Implements the IConfiger interface to send configuration bytes
 * through the StreamDock instance.
 */
#pragma once
#include "iconfiger.h"
#include "nullconfiger.h"

class Configer : public IConfiger
{
public:
	/**
	 * @brief Construct a Configer with a bound StreamDock instance.
	 * @param instance Pointer to the parent StreamDock.
	 */
	Configer(StreamDock* instance);
	~Configer();

	/**
	 * @brief Send the given configuration command to the device.
	 * @param configs Configuration byte vector.
	 */
	virtual void setDeviceConfig(std::vector<uint8_t> configs) override;

private:
	StreamDock* _instance = nullptr; ///< Reference to the associated StreamDock instance.
};
