/**
 * @file ireadcontroller.h
 * @brief Interface for handling device read operations and event dispatching.
 *
 * The IReadController interface allows polling or listening to device data.
 * It supports both raw data callbacks and decoded event callbacks.
 */
#pragma once
#include <cstdint>
#include <streamdock.h>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <condition_variable>

class StreamDock;
class IReadController
{
public:
	using ReadCallback = std::function<void()>; ///< Callback for specific key or knob event.
	using RawReadCallback = std::function<void(const std::vector<uint8_t>&)>; ///< Callback for raw data stream.

	IReadController() = default;
	virtual ~IReadController() = default;

	/**
	 * @brief Read data from the device (blocking or with timeout).
	 * @param timeoutMs Timeout in milliseconds (-1 for blocking).
	 * @return Raw data received from the device.
	 */
	virtual std::vector<uint8_t> read(int32_t timeoutMs = -1) = 0;

	/**
	 * @brief Register a decoded event callback for a specific key/knob value and event type.
	 * @param realValue Logical key/knob value.
	 * @param callback Callback function to trigger.
	 * @param event Optional event type (default: RegisterEvent::EveryThing).
	 * @param callbackAsync Whether to invoke the callback asynchronously.
	 */
	virtual void registerReadCallback(uint8_t realValue, ReadCallback callback, RegisterEvent event = RegisterEvent::EveryThing, bool callbackAsync = false) = 0;

	/**
	 * @brief Unregister a specific decoded event callback.
	 * @param realValue Logical key/knob value.
	 * @param event Event type to unregister.
	 */
	virtual void unregisterReadCallback(uint8_t realValue, RegisterEvent event) = 0;

	/**
	 * @brief Register a raw data callback (entire HID report).
	 * @param callback Function to handle raw data.
	 * @param callbackAsync Whether to invoke the callback asynchronously.
	 */
	virtual void registerRawReadCallback(RawReadCallback callback, bool callbackAsync = false) = 0;

	/**
	 * @brief Unregister the raw data callback.
	 */
	virtual void unregisterRawReadCallback() = 0;

	/**
	 * @brief Start the internal reading thread.
	 */
	virtual void startReadLoop() = 0;

	/**
	 * @brief Stop the internal reading thread.
	 */
	virtual void stopReadLoop() = 0;

	/**
	 * @brief Internal read loop function, to be run on a worker thread.
	 */
	virtual void readLoop() = 0;
};