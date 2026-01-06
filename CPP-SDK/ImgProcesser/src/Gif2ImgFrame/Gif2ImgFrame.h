#pragma once
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include "ImgType.h"
#include "IImageEncoder.h"

#define USE_THREADPOOL
#define DEBUG_TIME

// GIF frame data structure (includes encoded data and delay time)
struct GifFrameData {
    std::vector<uint8_t> encodedData;  // Encoded frame data
    uint16_t delayMs;                   // Frame delay (ms)
};

class Gif2ImgFrame {
public:
	explicit Gif2ImgFrame(const std::string& gifPath, std::shared_ptr<IImageEncoder> encoder);
	~Gif2ImgFrame();

	bool isValid() const;

	// Save each frame as an image file, output by type
	bool saveFramesToFiles(const std::string& outputDir, int quality = 95, const ImgHelper& imgHelper = ImgHelper());

	// Encode each frame as in-memory image data
	std::vector<std::vector<uint8_t>> encodeFramesToMemory(int quality = 95, const ImgHelper& imgHelper = ImgHelper());

	// Encode each frame as in-memory image data and include per-frame delay
	std::vector<GifFrameData> encodeFramesWithDelay(int quality = 95, const ImgHelper& imgHelper = ImgHelper());

private:
	struct Impl;
	Impl* impl_ = nullptr;
	std::shared_ptr<IImageEncoder> _encoder = nullptr;
};
