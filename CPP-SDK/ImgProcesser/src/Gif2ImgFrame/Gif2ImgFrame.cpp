#include "Gif2ImgFrame.h"
#include "RawCanvas.h"
#include <gif_lib.h>
#include <iostream>
#include <filesystem>
#include <future>
#include <chrono>
#include <iostream>
#include "ThreadPool.h"

#define DEFAULT_DISPOSAL_MODE DISPOSE_DO_NOT
#define FIRST_FRAME_DISPOSAL_MODE DISPOSE_DO_NOT
struct Gif2ImgFrame::Impl
{
	std::string path;
	GifFileType *gif = nullptr;
	int width = 0, height = 0;
	bool valid = false;

	Impl(const std::string &gifPath) : path(gifPath)
	{
		int error = 0;
		gif = DGifOpenFileName(gifPath.c_str(), &error);
		if (!gif || DGifSlurp(gif) != GIF_OK)
		{
			std::cerr << "读取 GIF 失败: " << gifPath << std::endl;
			if (gif)
				DGifCloseFile(gif, &error);
			gif = nullptr;
			return;
		}
		width = gif->SWidth;
		height = gif->SHeight;
		valid = true;
	}

	~Impl()
	{
		if (gif)
		{
			int error = 0;
			DGifCloseFile(gif, &error);
		}
	}

	void renderFrameRaw(int i, RawCanvas &canvas, int frameDisposalMode)
	{
		SavedImage &frame = gif->SavedImages[i];
		GifImageDesc &desc = frame.ImageDesc;
		ColorMapObject *cmap = desc.ColorMap ? desc.ColorMap : gif->SColorMap;
		if (!cmap)
			return;

		GraphicsControlBlock gcb{};
		DGifSavedExtensionToGCB(gif, i, &gcb);

		std::vector<uint8_t> backup = canvas.pixels; // 用于 DISPOSE_PREVIOUS

		if (frameDisposalMode == DISPOSE_BACKGROUND)
		{
			for (int y = 0; y < desc.Height; ++y)
			{
				for (int x = 0; x < desc.Width; ++x)
				{
					int gifX = desc.Left + x;
					int gifY = desc.Top + y;
					uint8_t *p = canvas.pixel(gifX, gifY);
					p[0] = p[1] = p[2] = 0;
					if (canvas.alpha)
						p[3] = 0;
				}
			}
		}
		else if (frameDisposalMode == DISPOSE_PREVIOUS)
		{
			canvas.pixels.swap(backup);
		}

		for (int y = 0; y < desc.Height; ++y)
		{
			for (int x = 0; x < desc.Width; ++x)
			{
				int gifX = desc.Left + x;
				int gifY = desc.Top + y;
				int idx = y * desc.Width + x;
				GifByteType colorIdx = frame.RasterBits[idx];

				if (canvas.alpha)
				{
					if (colorIdx == gcb.TransparentColor)
					{
						if (gcb.DisposalMode == DISPOSE_BACKGROUND)
						{
							uint8_t *p = canvas.pixel(gifX, gifY);
							p[0] = p[1] = p[2] = p[3] = 0;
						}
						continue;
					}
					GifColorType c = cmap->Colors[colorIdx];
					uint8_t *p = canvas.pixel(gifX, gifY);
					p[0] = c.Blue;
					p[1] = c.Green;
					p[2] = c.Red;
					p[3] = 255;
				}
				else
				{
					if (colorIdx == gcb.TransparentColor)
						continue;
					GifColorType c = cmap->Colors[colorIdx];
					uint8_t *p = canvas.pixel(gifX, gifY);
					p[0] = c.Blue;
					p[1] = c.Green;
					p[2] = c.Red;
				}
			}
		}
	}
};

Gif2ImgFrame::Gif2ImgFrame(const std::string &gifPath, std::shared_ptr<IImageEncoder> encoder) : impl_(new Impl(gifPath)), _encoder(std::move(encoder))
{
}
Gif2ImgFrame::~Gif2ImgFrame()
{
	delete impl_;
}
bool Gif2ImgFrame::isValid() const
{
	return impl_ && impl_->valid;
}

bool Gif2ImgFrame::saveFramesToFiles(const std::string &outputDir, int quality, const ImgHelper &imgHelper)
{
	if (!isValid())
		return false;

	std::filesystem::create_directories(outputDir);

	bool alpha = IImageEncoder::supportsAlpha(imgHelper._imgType);
	auto ext = IImageEncoder::imgTypeToExt(imgHelper._imgType);

	RawCanvas canvas(impl_->width, impl_->height, alpha);

	for (int i = 0; i < impl_->gif->ImageCount; ++i)
	{
		if (!_encoder)
			return false;
		int frameDisposalMode = DEFAULT_DISPOSAL_MODE;
		if (0 != i)
		{
			GraphicsControlBlock gcb{};
			DGifSavedExtensionToGCB(impl_->gif, i - 1, &gcb);
			frameDisposalMode = gcb.DisposalMode;
		}
		else
		{
			frameDisposalMode = FIRST_FRAME_DISPOSAL_MODE;
		}
		impl_->renderFrameRaw(i, canvas, frameDisposalMode);
		std::string filename = outputDir + "/frame_" + std::to_string(i) + ext;
		_encoder->encodeToFile(filename, canvas, quality, imgHelper);
	}

	return true;
}

std::vector<std::vector<uint8_t>> Gif2ImgFrame::encodeFramesToMemory(int quality, const ImgHelper &imgHelper)
{
	std::vector<std::vector<uint8_t>> result(impl_->gif->ImageCount);
	if (!isValid())
		return result;

	bool alpha = IImageEncoder::supportsAlpha(imgHelper._imgType);
	auto ext = IImageEncoder::imgTypeToExt(imgHelper._imgType);
	int width = impl_->width;
	int height = impl_->height;

	RawCanvas canvas(width, height, alpha);
#ifdef DEBUG_TIME
	using namespace std::chrono;
	auto t_start = high_resolution_clock::now();
#endif
#ifdef USE_THREADPOOL
	ThreadPool pool; // 使用默认线程数
	std::vector<std::future<void>> futures;
	for (int i = 0; i < impl_->gif->ImageCount; ++i)
	{
		int frameDisposalMode = DEFAULT_DISPOSAL_MODE;
		if (!_encoder)
			return result;
		if (0 != i)
		{
			GraphicsControlBlock gcb{};
			DGifSavedExtensionToGCB(impl_->gif, i - 1, &gcb);
			frameDisposalMode = gcb.DisposalMode;
		}
		else
		{
			frameDisposalMode = FIRST_FRAME_DISPOSAL_MODE;
		}
		impl_->renderFrameRaw(i, canvas, frameDisposalMode);
		auto frameCopy = std::make_shared<RawCanvas>(canvas);

		futures.emplace_back(pool.enqueue([&, i, frameCopy]()
										  { _encoder->encodeToMemory(result[i], *frameCopy, quality, imgHelper); }));
	}
	for (auto &f : futures)
		f.get();
#ifdef DEBUG_TIME
	auto t_end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(t_end - t_start).count();
	std::cerr << "[并行] 编码总耗时: " << duration << " ms" << std::endl;
#endif

#else
	for (int i = 0; i < impl_->gif->ImageCount; ++i)
	{
		if (!_encoder)
			return result;
		int frameDisposalMode = DEFAULT_DISPOSAL_MODE;
		if (0 != i)
		{
			GraphicsControlBlock gcb{};
			DGifSavedExtensionToGCB(impl_->gif, i - 1, &gcb);
			frameDisposalMode = gcb.DisposalMode;
		}
		else
		{
			frameDisposalMode = FIRST_FRAME_DISPOSAL_MODE;
		}
		impl_->renderFrameRaw(i, canvas, frameDisposalMode);
		_encoder->encodeToMemory(result[i], canvas, quality, imgHelper);
	}
#ifdef DEBUG_TIME
	auto t_end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(t_end - t_start).count();
	std::cerr << "[串行] 编码总耗时: " << duration << " ms" << std::endl;
#endif

#endif
	return result;
}

std::vector<GifFrameData> Gif2ImgFrame::encodeFramesWithDelay(int quality, const ImgHelper &imgHelper)
{
	std::vector<GifFrameData> result(impl_->gif->ImageCount);
	if (!isValid())
		return result;

	bool alpha = IImageEncoder::supportsAlpha(imgHelper._imgType);
	int width = impl_->width;
	int height = impl_->height;

	RawCanvas canvas(width, height, alpha);

#ifdef DEBUG_TIME
	using namespace std::chrono;
	auto t_start = high_resolution_clock::now();
#endif

#ifdef USE_THREADPOOL
	ThreadPool pool;
	std::vector<std::future<void>> futures;
	for (int i = 0; i < impl_->gif->ImageCount; ++i)
	{
		if (!_encoder)
			return result;

		// 获取当前帧的延迟时间
		GraphicsControlBlock gcb{};
		DGifSavedExtensionToGCB(impl_->gif, i, &gcb);

		// GIF 延迟单位是 10ms，需要转换为毫秒
		uint16_t delayMs = gcb.DelayTime * 10;
		if (delayMs == 0) delayMs = 100; // 默认 100ms（如果 GIF 未指定）

		result[i].delayMs = delayMs;

		// 获取帧的 disposal mode
		int frameDisposalMode = DEFAULT_DISPOSAL_MODE;
		if (0 != i)
		{
			GraphicsControlBlock gcbPrev{};
			DGifSavedExtensionToGCB(impl_->gif, i - 1, &gcbPrev);
			frameDisposalMode = gcbPrev.DisposalMode;
		}
		else
		{
			frameDisposalMode = FIRST_FRAME_DISPOSAL_MODE;
		}

		impl_->renderFrameRaw(i, canvas, frameDisposalMode);
		auto frameCopy = std::make_shared<RawCanvas>(canvas);

		futures.emplace_back(pool.enqueue([&, i, frameCopy]()
										  {
											  _encoder->encodeToMemory(result[i].encodedData, *frameCopy, quality, imgHelper);
										  }));
	}
	for (auto &f : futures)
		f.get();
#ifdef DEBUG_TIME
	auto t_end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(t_end - t_start).count();
	std::cerr << "[并行] 编码总耗时（含延迟）: " << duration << " ms" << std::endl;
#endif

#else
	for (int i = 0; i < impl_->gif->ImageCount; ++i)
	{
		if (!_encoder)
			return result;

		// 获取当前帧的延迟时间
		GraphicsControlBlock gcb{};
		DGifSavedExtensionToGCB(impl_->gif, i, &gcb);

		// GIF 延迟单位是 10ms，需要转换为毫秒
		uint16_t delayMs = gcb.DelayTime * 10;
		if (delayMs == 0) delayMs = 100; // 默认 100ms（如果 GIF 未指定）

		result[i].delayMs = delayMs;

		// 获取帧的 disposal mode
		int frameDisposalMode = DEFAULT_DISPOSAL_MODE;
		if (0 != i)
		{
			GraphicsControlBlock gcbPrev{};
			DGifSavedExtensionToGCB(impl_->gif, i - 1, &gcbPrev);
			frameDisposalMode = gcbPrev.DisposalMode;
		}
		else
		{
			frameDisposalMode = FIRST_FRAME_DISPOSAL_MODE;
		}

		impl_->renderFrameRaw(i, canvas, frameDisposalMode);
		_encoder->encodeToMemory(result[i].encodedData, canvas, quality, imgHelper);
	}
#ifdef DEBUG_TIME
	auto t_end = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(t_end - t_start).count();
	std::cerr << "[串行] 编码总耗时（含延迟）: " << duration << " ms" << std::endl;
#endif

#endif

	return result;
}