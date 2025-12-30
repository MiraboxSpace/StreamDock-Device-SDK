/**
 * @file nullgifcontroller.h
 * @brief No-op implementation of IGifController.
 *
 * Used for devices that do not support GIF functionality. All methods do nothing.
 */
#pragma once
#include "igifcontroller.h"

class NullGifController : public IGifController
{
public:
	virtual void setKeyGifFile(const std::string&, uint8_t) override
	{
	}
	virtual void setKeyGifStream(const std::vector<std::string>&, const std::vector<uint16_t>&, uint8_t) override
	{
	}
	virtual void setKeyGifStream(const std::vector<std::vector<uint8_t>>&, const std::vector<uint16_t>&, uint8_t) override
	{
	}
	virtual void clearKeyGif(uint8_t) override
	{
	}
	virtual void setBackgroundGifFile(const std::string&, int16_t = 0, uint16_t = 0, uint8_t FBlayer = 0x00) override
	{
	}
	virtual void setBackgroundGifStream(const std::vector<std::string>&, const std::vector<uint16_t>&, int16_t = 0, uint16_t = 0, uint8_t FBlayer = 0x00) override
	{
	}
	virtual void setBackgroundGifStream(const std::vector<std::vector<uint8_t>>&, const std::vector<uint16_t>&, uint16_t = 0, uint16_t = 0, uint8_t FBlayer = 0x00) override
	{
	}
	virtual void clearBackgroundGifStream(uint8_t clearPostion)override
	{
	}
	virtual void clearBackgroundyGif() override
	{
	}
	virtual void startGifLoop() override
	{
	}
	virtual void stopGifLoop() override
	{
	}
	virtual void gifWorkLoop() override
	{
	}
	virtual bool gifWorkLoopStatus() override
	{
		return false;
	}
};