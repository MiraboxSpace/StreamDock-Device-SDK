#pragma once
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include "ImgType.h"
#include "IImageEncoder.h"

#define USE_THREADPOOL
#define DEBUG_TIME

// GIF 帧数据结构（包含编码后的数据和延迟时间）
struct GifFrameData {
    std::vector<uint8_t> encodedData;  // 编码后的帧数据
    uint16_t delayMs;                   // 帧延迟（毫秒）
};

class Gif2ImgFrame {
public:
	explicit Gif2ImgFrame(const std::string& gifPath, std::shared_ptr<IImageEncoder> encoder);
	~Gif2ImgFrame();

	bool isValid() const;

	// 保存每帧为图像文件，按类型输出
	bool saveFramesToFiles(const std::string& outputDir, int quality = 95, const ImgHelper& imgHelper = ImgHelper());

	// 将每帧编码为内存图像数据
	std::vector<std::vector<uint8_t>> encodeFramesToMemory(int quality = 95, const ImgHelper& imgHelper = ImgHelper());

	// 将每帧编码为内存图像数据，并包含每帧的延迟时间
	std::vector<GifFrameData> encodeFramesWithDelay(int quality = 95, const ImgHelper& imgHelper = ImgHelper());

private:
	struct Impl;
	Impl* impl_ = nullptr;
	std::shared_ptr<IImageEncoder> _encoder = nullptr;
};
