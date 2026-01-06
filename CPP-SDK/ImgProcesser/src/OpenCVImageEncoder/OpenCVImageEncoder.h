// ImageEncoder_OpenCV.h
#pragma once
#include "IImageEncoder.h"
#include <opencv2/opencv.hpp>

class OpenCVImageEncoder : public IImageEncoder
{
public:
	virtual bool encodeToFile(const std::string& filename,
		const RawCanvas& canvas,
		int quality,
		const ImgHelper& imgHelper) const override;

	virtual bool encodeToMemory(std::vector<uint8_t>& out,
		const RawCanvas& canvas,
		int quality,
		const ImgHelper& imgHelper) const override;

	virtual bool encodeToFile(const std::string& filename,
		std::vector<uint8_t>& out,
		int quality = 95,
		const ImgHelper& imgHelper = ImgHelper()) const override;

	virtual bool encodeToMemory(std::vector<uint8_t>& out,
		const std::vector<uint8_t>& in,
		int quality = 95,
		const ImgHelper& imgHelper = ImgHelper()) const override;

	virtual bool encodeToBitmap(std::vector<uint8_t>& out,
		const std::vector<uint8_t>& in,
		const ImgHelper& imgHelper = ImgHelper()) const override;

	static std::vector<int> imgEncodeParams(ImgType type, int quality);

	enum class FlipMode {
		Horizontal,  // Flip horizontally
		Vertical     // Flip vertically
	};
	void rotate(cv::Mat& mat, double angle) const;
	void flip(cv::Mat& mat, bool hflip, bool vflip) const;
	void crop(cv::Mat& mat, uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;
	bool convertMatToRawBytes(const cv::Mat& inputBGR, std::vector<uint8_t>&, ImgFormat format) const;
};

template <>
inline cv::Mat RawCanvas::as<cv::Mat>() const
{
	int type = alpha ? CV_8UC4 : CV_8UC3;
	return cv::Mat(height, width, type,
		const_cast<uint8_t*>(pixels.data()),
		width * (alpha ? 4 : 3));
}
