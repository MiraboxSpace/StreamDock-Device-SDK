#include "OpenCVImageEncoder.h"

bool OpenCVImageEncoder::encodeToFile(const std::string& filename,
	const RawCanvas& canvas,
	int quality,
	const ImgHelper& imgHelper) const
{
	cv::Mat input = canvas.as<cv::Mat>();
	if (input.empty()) return false;

	// 默认参数：直接写入
	if (imgHelper == ImgHelper())
		return cv::imwrite(filename, input, imgEncodeParams(ImgType::JPG, quality));

	// 参数提取
	int32_t crop_offset_x = static_cast<int32_t>(imgHelper._crop_offset_x);
	int32_t crop_offset_y = static_cast<int32_t>(imgHelper._crop_offset_y);
	uint32_t targetWidth = imgHelper._width;
	uint32_t targetHeight = imgHelper._height;
	double angle = imgHelper._rotateAngle;
	bool flipV = imgHelper._flipVertical;
	bool flipH = imgHelper._flipHorizonal;
	ResizeOption resizeOpt = imgHelper._resizeOption;

	cv::Mat processed;

	// 判断裁剪或缩放
	if (imgHelper._processer == ImgProcess::Crop && (crop_offset_x >= 0 && crop_offset_y >= 0 && targetWidth > 0 && targetHeight > 0)) {
		// 有裁剪参数，优先裁剪
		crop(input, crop_offset_x, crop_offset_y, targetWidth, targetHeight);
		processed = input;
	}
	else if (imgHelper._processer == ImgProcess::Resize && (targetWidth > 0 && targetHeight > 0)) {
		// 否则按缩放或填充
		if (resizeOpt == ResizeOption::Scale) {
			cv::resize(input, processed, cv::Size(targetWidth, targetHeight), 0, 0, cv::INTER_AREA);
		}
		else if (resizeOpt == ResizeOption::Pad) {
			double scale = std::min(
				static_cast<double>(targetWidth) / input.cols,
				static_cast<double>(targetHeight) / input.rows
			);
			cv::Mat scaled;
			cv::resize(input, scaled, cv::Size(), scale, scale, cv::INTER_AREA);

			int top = (targetHeight - scaled.rows) / 2;
			int bottom = targetHeight - scaled.rows - top;
			int left = (targetWidth - scaled.cols) / 2;
			int right = targetWidth - scaled.cols - left;

			cv::copyMakeBorder(scaled, processed, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
		}
		else {
			processed = input.clone(); // fallback
		}
	}
	else {
		// fallback 到原图
		processed = input.clone();
	}

	// 旋转 & 翻转
	rotate(processed, angle);
	flip(processed, flipH, flipV);

	// 写入文件
	return cv::imwrite(filename, processed, imgEncodeParams(imgHelper._imgType, quality));
}


bool OpenCVImageEncoder::encodeToMemory(std::vector<uint8_t>& out,
	const RawCanvas& canvas,
	int quality,
	const ImgHelper& imgHelper) const
{
	cv::Mat input = canvas.as<cv::Mat>();
	if (input.empty()) return false;

	if (imgHelper == ImgHelper())
		return cv::imencode(imgTypeToExt(ImgType::JPG), input, out, imgEncodeParams(ImgType::JPG, quality));

	// 获取参数
	int32_t crop_offset_x = static_cast<int32_t>(imgHelper._crop_offset_x);
	int32_t crop_offset_y = static_cast<int32_t>(imgHelper._crop_offset_y);
	uint32_t targetWidth = imgHelper._width;
	uint32_t targetHeight = imgHelper._height;
	double angle = imgHelper._rotateAngle;
	bool flipV = imgHelper._flipVertical;
	bool flipH = imgHelper._flipHorizonal;
	ResizeOption resizeOpt = imgHelper._resizeOption;

	cv::Mat processed;

	// 判断是否进行裁剪或缩放填充
	if (imgHelper._processer == ImgProcess::Crop && (crop_offset_x >= 0 && crop_offset_y >= 0 && targetWidth > 0 && targetHeight > 0)) {
		// 裁剪区域不为空 -> crop
		crop(input, crop_offset_x, crop_offset_y, targetWidth, targetHeight);
		processed = input;
	}
	else if (imgHelper._processer == ImgProcess::Resize && (targetWidth > 0 && targetHeight > 0)) {
		// 无裁剪偏移 -> Resize
		if (resizeOpt == ResizeOption::Scale) {
			cv::resize(input, processed, cv::Size(targetWidth, targetHeight), 0, 0, cv::INTER_AREA);
		}
		else if (resizeOpt == ResizeOption::Pad) {
			double scale = std::min(
				static_cast<double>(targetWidth) / input.cols,
				static_cast<double>(targetHeight) / input.rows
			);
			cv::Mat scaled;
			cv::resize(input, scaled, cv::Size(), scale, scale, cv::INTER_AREA);

			int top = (targetHeight - scaled.rows) / 2;
			int bottom = targetHeight - scaled.rows - top;
			int left = (targetWidth - scaled.cols) / 2;
			int right = targetWidth - scaled.cols - left;

			cv::copyMakeBorder(scaled, processed, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
		}
		else {
			processed = input.clone(); // fallback
		}
	}
	else {
		// fallback to original
		processed = input.clone();
	}

	// 应用旋转 & 翻转
	rotate(processed, angle);
	flip(processed, flipH, flipV);

	// 编码为图像字节流
	return cv::imencode(imgTypeToExt(imgHelper._imgType), processed, out, imgEncodeParams(imgHelper._imgType, quality));
}


bool OpenCVImageEncoder::encodeToFile(const std::string& filename,
	std::vector<uint8_t>& in,
	int quality,
	const ImgHelper& imgHelper) const
{
	cv::Mat input = cv::imdecode(in, cv::IMREAD_COLOR);
	if (input.empty()) return false;

	// 重用已有流程
	if (imgHelper == ImgHelper())
		return cv::imwrite(filename, input, imgEncodeParams(ImgType::JPG, quality));

	// 获取参数
	uint32_t targetWidth = imgHelper._width;
	uint32_t targetHeight = imgHelper._height;
	double angle = imgHelper._rotateAngle;
	bool flipV = imgHelper._flipVertical;
	bool flipH = imgHelper._flipHorizonal;
	ResizeOption resizeOpt = imgHelper._resizeOption;

	cv::Mat resized;
	if (resizeOpt == ResizeOption::Scale) {
		cv::resize(input, resized, cv::Size(targetWidth, targetHeight), 0, 0, cv::INTER_AREA);
	}
	else if (resizeOpt == ResizeOption::Pad) {
		double scale = std::min(static_cast<double>(targetWidth) / input.cols,
			static_cast<double>(targetHeight) / input.rows);
		cv::Mat scaled;
		cv::resize(input, scaled, cv::Size(), scale, scale, cv::INTER_AREA);

		int top = (targetHeight - scaled.rows) / 2;
		int bottom = targetHeight - scaled.rows - top;
		int left = (targetWidth - scaled.cols) / 2;
		int right = targetWidth - scaled.cols - left;

		cv::copyMakeBorder(scaled, resized, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
	}
	else {
		resized = input.clone();
	}

	rotate(resized, angle);
	flip(resized, flipH, flipV);

	return cv::imwrite(filename, resized, imgEncodeParams(imgHelper._imgType, quality));
}

bool OpenCVImageEncoder::encodeToMemory(std::vector<uint8_t>& out,
	const std::vector<uint8_t>& in,
	int quality,
	const ImgHelper& imgHelper) const
{
	cv::Mat input = cv::imdecode(in, cv::IMREAD_COLOR);
	if (input.empty()) return false;

	if (imgHelper == ImgHelper())
		return cv::imencode(imgTypeToExt(ImgType::JPG), input, out, imgEncodeParams(ImgType::JPG, quality));

	if (imgHelper._imgType == ImgType::RAW)   /// 如果需要原始数据, 直接 encodeToBitmap
		return encodeToBitmap(out, in, imgHelper);

	// 获取参数
	uint32_t targetWidth = imgHelper._width;
	uint32_t targetHeight = imgHelper._height;
	double angle = imgHelper._rotateAngle;
	bool flipV = imgHelper._flipVertical;
	bool flipH = imgHelper._flipHorizonal;
	ResizeOption resizeOpt = imgHelper._resizeOption;

	cv::Mat resized;
	if (resizeOpt == ResizeOption::Scale) {
		cv::resize(input, resized, cv::Size(targetWidth, targetHeight), 0, 0, cv::INTER_AREA);
	}
	else if (resizeOpt == ResizeOption::Pad) {
		double scale = std::min(static_cast<double>(targetWidth) / input.cols,
			static_cast<double>(targetHeight) / input.rows);
		cv::Mat scaled;
		cv::resize(input, scaled, cv::Size(), scale, scale, cv::INTER_AREA);

		int top = (targetHeight - scaled.rows) / 2;
		int bottom = targetHeight - scaled.rows - top;
		int left = (targetWidth - scaled.cols) / 2;
		int right = targetWidth - scaled.cols - left;

		cv::copyMakeBorder(scaled, resized, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
	}
	else {
		resized = input.clone();
	}

	rotate(resized, angle);
	flip(resized, flipH, flipV);

	return cv::imencode(imgTypeToExt(imgHelper._imgType), resized, out, imgEncodeParams(imgHelper._imgType, quality));
}
#include <fstream>

bool saveToFile(const std::string& filename, const std::vector<uint8_t>& out)
{
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	file.write(reinterpret_cast<const char*>(out.data()), out.size());
	file.close();

	return true;
}

bool OpenCVImageEncoder::encodeToBitmap(std::vector<uint8_t>& out,
	const std::vector<uint8_t>& in,
	const ImgHelper& imgHelper) const
{
	cv::Mat input = cv::imdecode(in, cv::IMREAD_COLOR);

	if (input.empty()) return false;

	cv::Mat processed;

	if (imgHelper == ImgHelper()) {
		processed = input;
	}
	else {
		uint32_t targetWidth = imgHelper._width;
		uint32_t targetHeight = imgHelper._height;
		double angle = imgHelper._rotateAngle;
		bool flipV = imgHelper._flipVertical;
		bool flipH = imgHelper._flipHorizonal;
		ResizeOption resizeOpt = imgHelper._resizeOption;

		if (resizeOpt == ResizeOption::Scale) {
			cv::resize(input, processed, cv::Size(targetWidth, targetHeight), 0, 0, cv::INTER_AREA);
		}
		else if (resizeOpt == ResizeOption::Pad) {
			double scale = std::min(static_cast<double>(targetWidth) / input.cols,
				static_cast<double>(targetHeight) / input.rows);
			cv::Mat scaled;
			cv::resize(input, scaled, cv::Size(), scale, scale, cv::INTER_AREA);

			int top = (targetHeight - scaled.rows) / 2;
			int bottom = targetHeight - scaled.rows - top;
			int left = (targetWidth - scaled.cols) / 2;
			int right = targetWidth - scaled.cols - left;

			cv::copyMakeBorder(scaled, processed, top, bottom, left, right,
				cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
		}
		else {
			processed = input.clone();
		}

		rotate(processed, angle);
		flip(processed, flipH, flipV);
	}

	if (!processed.isContinuous()) {
		processed = processed.clone(); // 确保内存连续
	}

	convertMatToRawBytes(processed, out, imgHelper._imgFormat);
	return true;
}


std::vector<int> OpenCVImageEncoder::imgEncodeParams(ImgType type, int quality)
{
	switch (type)
	{
	case ImgType::JPG:
		return { cv::IMWRITE_JPEG_QUALITY, quality };
	case ImgType::PNG:
		return { cv::IMWRITE_PNG_COMPRESSION, 3 };
	case ImgType::WEBP:
		return { cv::IMWRITE_WEBP_QUALITY, quality };
	default:
		return {};
	}
}

void OpenCVImageEncoder::rotate(cv::Mat& mat, double angle) const
{
	if (mat.empty()) return;

	// 归一化角度到 [0, 360)
	int normalizedAngle = static_cast<int>(angle) % 360;
	if (normalizedAngle < 0) normalizedAngle += 360;

	// 特殊处理 90° 倍数
	if (normalizedAngle == 90) {
		cv::rotate(mat, mat, cv::ROTATE_90_CLOCKWISE);
		return;
	}
	else if (normalizedAngle == 180) {
		cv::rotate(mat, mat, cv::ROTATE_180);
		return;
	}
	else if (normalizedAngle == 270) {
		cv::rotate(mat, mat, cv::ROTATE_90_COUNTERCLOCKWISE);
		return;
	}

	// 一般角度使用 warpAffine，保持尺寸不变
	cv::Point2f center(mat.cols / 2.0f, mat.rows / 2.0f);
	cv::Mat rotationMat = cv::getRotationMatrix2D(center, angle, 1.0);
	cv::Mat rotated;
	cv::warpAffine(mat, rotated, rotationMat, mat.size(), cv::INTER_LINEAR, cv::BORDER_REPLICATE);
	mat = rotated;
}

void OpenCVImageEncoder::flip(cv::Mat& mat, bool hflip, bool vflip) const
{
	if (mat.empty()) return;

	// 不翻转，直接返回
	if (!hflip && !vflip) return;

	int flipCode = 0;
	if (hflip && vflip) {
		flipCode = -1;  // 水平 + 垂直（等价于旋转180° + 镜像）
	}
	else if (hflip) {
		flipCode = 1;   // 仅水平
	}
	else if (vflip) {
		flipCode = 0;   // 仅垂直
	}

	cv::Mat flipped;
	cv::flip(mat, flipped, flipCode);
	mat = flipped;
}

void OpenCVImageEncoder::crop(cv::Mat& mat, uint32_t x, uint32_t y, uint32_t width, uint32_t height) const
{
	if (mat.empty()) return;

	int x1 = static_cast<int>(x);
	int y1 = static_cast<int>(y);
	int x2 = static_cast<int>(x + width);
	int y2 = static_cast<int>(y + height);

	x1 = std::clamp(x1, 0, mat.cols);
	y1 = std::clamp(y1, 0, mat.rows);
	x2 = std::clamp(x2, 0, mat.cols);
	y2 = std::clamp(y2, 0, mat.rows);

	int w = x2 - x1;
	int h = y2 - y1;

	if (w <= 0 || h <= 0) return;

	// 裁剪
	cv::Rect roi(x1, y1, w, h);
	mat = mat(roi).clone();
}

bool OpenCVImageEncoder::convertMatToRawBytes(const cv::Mat& inputBGR, std::vector<uint8_t>& out, ImgFormat format) const {
	if (inputBGR.empty()) return false;

	switch (format)
	{
	case ImgFormat::BGR888:
	{
		//if (inputBGR.channels() != 3 || inputBGR.type() != CV_8UC3)
		//    return false;
		out.assign(inputBGR.datastart, inputBGR.dataend);
		break;
	}
	case ImgFormat::RGB16:
	{
		cv::Mat rgb;
		cv::cvtColor(inputBGR, rgb, cv::COLOR_BGR2RGB);
		cv::Mat rgb565;
		cv::cvtColor(rgb, rgb565, cv::COLOR_RGB2BGR565);
		out.assign(rgb565.datastart, rgb565.dataend);
		break;
	}
	case ImgFormat::RGB888:
	{
		break;
	}
	default:
		return false;
	}
	return true;
}
