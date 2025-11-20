/**
 * @file iheartbeat.h
 * @brief Defines the interface for heartbeat functionality in StreamDock devices.
 *
 * This interface provides virtual methods for starting and stopping a heartbeat loop,
 * which can be used to periodically notify the device or maintain connection state.
 */
#pragma once
#include <cstdint>
#include <streamdock.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

class StreamDock;
class IHeartBeat
{
public:
	IHeartBeat() = default;
	virtual ~IHeartBeat() = default;
	/**
	 * @brief Start the heartbeat loop in a background thread.
	 */
	virtual void startHeartBeatLoop() = 0;

	/**
	 * @brief Stop the running heartbeat loop.
	 */
	virtual void stopHeartBeatLoop() = 0;

	/**
	 * @brief Worker loop function executed in a thread.
	 */
	virtual void heartBeatLoop() = 0;
};
