/**
 * @file nullconfiger.h
 * @brief Null implementation of IConfiger (no-op).
 *
 * Used as a fallback when a device does not support configuration features.
 */
#pragma once
#include "iconfiger.h"

class NullConfiger : public IConfiger
{
public:
	/**
	 * @brief No-op implementation; does nothing.
	 * @param configs Ignored.
	 */
	virtual void setDeviceConfig(std::vector<uint8_t> configs) override {
	}
};
