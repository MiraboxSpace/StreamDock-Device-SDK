/**
 * @file heartbeat.h
 * @brief Concrete implementation of IHeartBeat that periodically sends heartbeat signals to the device.
 *
 * This class uses a background worker thread to call the device's `heartbeat()` method
 * at a fixed interval (defined by `HEART_BEAT_TIME`).
 */
#pragma once
#include "iheartbeat.h"
#include "nullheartbeat.h"

constexpr const auto HEART_BEAT_TIME = 10;

class HeartBeat : public IHeartBeat
{
public:
	/**
	 * @brief Construct the heartbeat controller with a given StreamDock device.
	 * @param instance Pointer to the StreamDock device.
	 */
	HeartBeat(StreamDock* instance);
	~HeartBeat();

	/// @copydoc IHeartBeat::startHeartBeatLoop
	virtual void startHeartBeatLoop() override;

	/// @copydoc IHeartBeat::stopHeartBeatLoop
	virtual void stopHeartBeatLoop() override;

	/// @copydoc IHeartBeat::heartBeatLoop
	virtual void heartBeatLoop() override;

private:
	/**
	 * @brief Launch the worker thread for the heartbeat loop.
	 */
	void startWorkerThread();

	/**
	 * @brief Stop the running heartbeat worker thread.
	 */
	void stopWorkerThread();

private:
	StreamDock* _instance = nullptr;      ///< Pointer to the parent StreamDock device.
	std::thread _heartbeatThread;         ///< Worker thread for the heartbeat loop.
	std::atomic<bool> _running = false;   ///< Flag indicating whether the thread is running.
	std::atomic<bool> _HeartBeatLoopEnabled = false; ///< Flag controlling whether heartbeat loop should continue.
	std::mutex _heartbeatMutex;           ///< Mutex for synchronizing heartbeat thread state.
	std::condition_variable _heartbeatCv; ///< Condition variable for worker sleep/wakeup.
};
