/**
 * @file igifcontroller.h
 * @brief Interface for controlling GIF animations on StreamDock devices.
 *
 * This interface defines methods for setting GIFs on keys and backgrounds,
 * controlling playback, and managing worker threads for animation timing.
 */
#pragma once
#include <streamdock.h>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class StreamDock;
class IGifController
{
public:
	virtual ~IGifController() = default;
	/**
	 * @brief Set a key GIF from a file path.
	 */
	virtual void setKeyGifFile(const std::string& gifStream, uint8_t keyValue, uint16_t gifDelay = 100) = 0;

	/**
	 * @brief Set a key GIF from frame strings.
	 */
	virtual void setKeyGifStream(const std::vector<std::string>& gifStream, uint8_t keyValue, uint16_t gifDelay = 100) = 0;

	/**
	 * @brief Set a key GIF from raw image frame data.
	 */
	virtual void setKeyGifStream(const std::vector<std::vector<uint8_t>>& gifStream, uint8_t keyValue, uint16_t gifDelay = 100) = 0;

	/**
	 * @brief Set a background GIF from file.
	 */
	virtual void setBackgroundGifFile(const std::string& gifPath, int16_t background_place_x = 0, uint16_t background_place_y = 0, uint16_t gifDelay = 100, uint8_t FBlayer = 0x00) = 0;

	/**
	 * @brief Set a background GIF from frames strings.
	 */
	virtual void setBackgroundGifStream(const std::vector<std::string>& gifStream, int16_t background_place_x = 0, uint16_t background_place_y = 0, uint16_t gifDelay = 100, uint8_t FBlayer = 0x00) = 0;

	/**
	 * @brief Set a background GIF from raw frame data.
	 */
	virtual void setBackgroundGifStream(const std::vector<std::vector<uint8_t>>& gifStream, uint16_t background_place_x = 0, uint16_t background_place_y = 0, uint16_t gifDelay = 100, uint8_t FBlayer = 0x00) = 0;

	/**
	 * @brief Clear specific background position GIF.
	 */
	virtual void clearBackgroundGifStream(uint8_t clearPostion = 0x03) = 0;

	/**
	 * @brief Clear the key GIF on the specified key.
	 */
	virtual void clearKeyGif(uint8_t keyValue) = 0;

	/**
	 * @brief Clear all background GIFs.
	 */
	virtual void clearBackgroundyGif() = 0;

	/**
	 * @brief Start the GIF worker loop.
	 */
	virtual void startGifLoop() = 0;

	/**
	 * @brief Stop the GIF worker loop.
	 */
	virtual void stopGifLoop() = 0;

	/**
	 * @brief Worker loop for processing and updating GIF frames.
	 */
	virtual void gifWorkLoop() = 0;

	/**
	 * @brief Query whether the GIF loop is currently running.
	 */
	virtual bool gifWorkLoopStatus() = 0;

protected:
	const uint16_t _baseGifDelayMs = 100;  ///< Default delay between GIF frames.
};