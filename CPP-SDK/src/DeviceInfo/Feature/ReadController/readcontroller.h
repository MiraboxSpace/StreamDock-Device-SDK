/**
 * @file readcontroller.h
 * @brief Implementation of IReadController for reading and dispatching events from StreamDock.
 *
 * This controller handles data polling from the device, mapping hardware values to logical keys,
 * and invoking registered callbacks.
 */
#pragma once
#include "ireadcontroller.h"
#include "nullreadcontroller.h"

class ReadController : public IReadController
{
public:
	/**
	 * @brief Constructor.
	 * @param instance Pointer to the StreamDock instance.
	 */
	ReadController(StreamDock* instance);

	/**
	 * @brief Destructor.
	 */
	~ReadController();

	/// @copydoc IReadController::read
	virtual std::vector<uint8_t> read(int32_t timeoutMs = -1) override;

	/// @copydoc IReadController::registerReadCallback
	virtual void registerReadCallback(uint8_t realValue, IReadController::ReadCallback callback, RegisterEvent event = RegisterEvent::EveryThing, bool callbackAsync = false) override;

	/// @copydoc IReadController::unregisterReadCallback
	virtual void unregisterReadCallback(uint8_t realValue, RegisterEvent event) override;

	/// @copydoc IReadController::registerRawReadCallback
	virtual void registerRawReadCallback(IReadController::RawReadCallback callback, bool callbackAsync = false) override;

	/// @copydoc IReadController::unregisterRawReadCallback
	virtual void unregisterRawReadCallback() override;

	/// @copydoc IReadController::startReadLoop
	virtual void startReadLoop() override;

	/// @copydoc IReadController::stopReadLoop
	virtual void stopReadLoop() override;

	/// @copydoc IReadController::readLoop
	virtual void readLoop() override;

private:
	/**
	 * @brief Launch the worker thread for reading data.
	 */
	void startWorkerThread();

	/**
	 * @brief Stop the worker thread.
	 */
	void stopWorkerThread();

private:
	StreamDock* _instance = nullptr; ///< StreamDock device instance.
	std::thread _readThread;         ///< Read loop worker thread.
	std::atomic<bool> _running = false; ///< Whether the worker thread is active.
	std::atomic<bool> _readLoopEnabled = false; ///< Whether the read loop should continue running.
	std::mutex _readMutex;
	std::condition_variable _readCv;

	struct PairHash {
		std::size_t operator()(const std::pair<uint8_t, RegisterEvent>& p) const {
			return std::hash<uint8_t>()(p.first) ^ (std::hash<int>()(static_cast<int>(p.second)) << 1);
		}
	};

	struct CallbackStructure {
		ReadCallback callback = nullptr;
		bool async_ = false;
	};

	/// Mapping from (logical key, event) to callback structure.
	std::unordered_map<std::pair<uint8_t, RegisterEvent>, CallbackStructure, PairHash> _readCallbackMap;

	struct RawReadCallbackStructure {
		RawReadCallback callback = nullptr;
		bool async_ = false;
	};

	RawReadCallbackStructure _rawReadCallback = { nullptr, false }; ///< Raw read callback configuration.
};