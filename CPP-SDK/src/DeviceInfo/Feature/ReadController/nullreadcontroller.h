/**
 * @file nullreadcontroller.h
 * @brief Null implementation of IReadController for devices with no read support.
 *
 * All methods are implemented as no-ops. Safe to use as a fallback.
 */
#pragma once
#include "ireadcontroller.h"

class NullReadController : public IReadController
{
public:
	NullReadController() = default;
	virtual ~NullReadController() = default;
	virtual std::vector<uint8_t> read(int32_t timeoutMs = -1) override
	{
		return {};
	};
	virtual void registerReadCallback(uint8_t realValue, IReadController::ReadCallback callback, RegisterEvent event = RegisterEvent::EveryThing, bool callbackAsync = false) override
	{
	}
	virtual void unregisterReadCallback(uint8_t realValue, RegisterEvent event) override
	{
	}
	virtual void registerRawReadCallback(IReadController::RawReadCallback callback, bool callbackAsync = false) override
	{
	}
	virtual void unregisterRawReadCallback() override
	{
	}
	virtual void startReadLoop() override
	{
	}
	virtual void stopReadLoop() override
	{
	}
	virtual void readLoop() override
	{
	}
};