#pragma once
#include <cstdint>
#include <ImgType.h>

enum class ImgProcess : uint8_t {
	Resize,// 重置大小到目标
	Crop,// 裁剪大小
};

enum class ResizeOption : uint8_t {
	Scale,// 缩放
	Pad,// 填充
};

class ImgHelper
{
public:
	ImgHelper() = default;
	ImgHelper(uint32_t width, uint32_t height, double rotateAngle, ResizeOption resizeOption = ResizeOption::Scale,
		bool flipVertical = false, bool flipHorizonal = false, ImgType type = ImgType::JPG, ImgFormat format = ImgFormat::BGR888)
		:_width(width), _height(height), _rotateAngle(rotateAngle), _resizeOption(resizeOption),
		_flipVertical(flipVertical), _flipHorizonal(flipHorizonal), _imgType(type), _imgFormat(format) {
	}
	ImgHelper(int32_t x, int32_t y, uint32_t width, uint32_t height, double rotateAngle, ResizeOption resizeOption = ResizeOption::Scale,
		bool flipVertical = false, bool flipHorizonal = false, ImgType type = ImgType::JPG, ImgFormat format = ImgFormat::BGR888)
		:_crop_offset_x(x), _crop_offset_y(y), _width(width), _height(height), _rotateAngle(rotateAngle), _resizeOption(resizeOption),
		_flipVertical(flipVertical), _flipHorizonal(flipHorizonal), _imgType(type), _imgFormat(format) {
	}
	~ImgHelper() = default;
	bool operator==(const ImgHelper& other) const {
		return _crop_offset_x == other._crop_offset_x &&
			_crop_offset_y == other._crop_offset_y &&
			_width == other._width &&
			_height == other._height &&
			_rotateAngle == other._rotateAngle &&
			_resizeOption == other._resizeOption &&
			_flipVertical == other._flipVertical &&
			_flipHorizonal == other._flipHorizonal &&
			_imgType == other._imgType &&
			_imgFormat == other._imgFormat;
	}

	bool operator!=(const ImgHelper& other) const {
		return !(*this == other);
	}

public:
	int32_t _crop_offset_x = -1;
	int32_t _crop_offset_y = -1;
	uint32_t _width = 0;
	uint32_t _height = 0;
	double _rotateAngle = 0.0f;
	ResizeOption _resizeOption = ResizeOption::Scale;
	bool _flipVertical = false;
	bool _flipHorizonal = false;
	ImgType _imgType = ImgType::JPG;
	ImgFormat _imgFormat = ImgFormat::BGR888;
	ImgProcess _processer = ImgProcess::Resize;
};