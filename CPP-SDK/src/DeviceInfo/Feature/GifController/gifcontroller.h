#pragma once
#include "igifcontroller.h"
#include "nullgifcontroller.h"

class GifController : public IGifController
{
public:
	GifController(StreamDock* instance);
	~GifController();

public:
	/// Overridden GIF control methods (see IGifController for docs)
	virtual void setKeyGifFile(const std::string& gifPath, uint8_t keyValue) override;
	virtual void setKeyGifStream(const std::vector<std::string>& gifStream, const std::vector<uint16_t>& frameDelays, uint8_t keyValue) override;
	virtual void setKeyGifStream(const std::vector<std::vector<uint8_t>>& gifStream, const std::vector<uint16_t>& frameDelays, uint8_t keyValue) override;
	virtual void setBackgroundGifFile(const std::string& gifPath, int16_t background_place_x = 0, uint16_t background_place_y = 0, uint8_t FBlayer = 0x00) override;
	virtual void setBackgroundGifStream(const std::vector<std::string>& gifStream, const std::vector<uint16_t>& frameDelays, int16_t background_place_x = 0, uint16_t background_place_y = 0, uint8_t FBlayer = 0x00) override;
	virtual void setBackgroundGifStream(const std::vector<std::vector<uint8_t>>& gifStream, const std::vector<uint16_t>& frameDelays, uint16_t background_place_x = 0, uint16_t background_place_y = 0, uint8_t FBlayer = 0x00) override;
	virtual void clearBackgroundGifStream(uint8_t clearPostion = 0x03) override;
	virtual void clearKeyGif(uint8_t keyValue) override;
	virtual void clearBackgroundyGif() override;
	virtual void startGifLoop() override;
	virtual void stopGifLoop() override;
	virtual void gifWorkLoop() override;
	virtual bool gifWorkLoopStatus() override;

private:
	/**
	 * @brief Start the internal worker thread for GIF playback.
	 */
	void startWorkerThread();

	/**
	 * @brief Stop the worker thread.
	 */
	void stopWorkerThread();

	/**
	 * @brief Draw a single frame to the background framebuffer.
	 */
	void setBackgroundGifFileFrame(const std::string& stream, uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, uint8_t FBlayer = 0x00);

	/**
	 * @brief Clear background framebuffer by position.
	 */
	void clearBackgroundGifFileFrame(uint8_t clearPostion = 0x03);

private:
	StreamDock* _instance = nullptr;                ///< Parent device instance.
	/// gif status
	using GifStreamType = std::vector<std::string>;
	std::atomic<bool> _running = false;        ///< Worker thread running flag.
	std::atomic<bool> _gifLoopEnabled = false; ///< Whether GIF looping is enabled.
	std::mutex _gifMutex;                      ///< Mutex for GIF operations.
	std::condition_variable _gifCv;            ///< Condition variable for GIF worker wake-up.
	std::thread _gifThread;                    ///< Worker thread for GIF playback.
	struct GifStreamStatus
	{
		GifStreamType gifFrames;
		std::vector<uint16_t> frameDelays;  // 每帧的延迟（毫秒）
		size_t currentFrame = 0;
		uint64_t accumulatedTime = 0;       // 累积时间（微秒）
	};
	std::unordered_map<uint8_t, GifStreamStatus> _gifMap; ///< Key GIF frame buffers.
	uint16_t _background_place_x = 0; ///< X-position for background GIF.
	uint16_t _background_place_y = 0; ///< Y-position for background GIF.

	// 自适应时序控制（用于优化GIF播放流畅度）
	bool _enableAdaptiveTiming = true;  ///< 是否启用自适应延迟补偿
	bool _enablePerformanceLogging = false;  ///< 是否启用性能日志（调试用）
};
