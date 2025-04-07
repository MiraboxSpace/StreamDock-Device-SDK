#include "StreamDockN1.h"

StreamDockN1::StreamDockN1(const std::shared_ptr<TranSport>& transport, struct hid_device_info* copyDevInfo) : StreamDock(transport, copyDevInfo)
{
	setKeyFormat(96, 96);
	setKeyFormat_secondScreen(64, 64);
	setKeyRange(1, 15);
	setKeySecondScreenRange(16, 18);
}

StreamDockN1::~StreamDockN1()
{
}

void StreamDockN1::init(int brightness)
{
	this->transport->switchMode(mode);
	StreamDock::init();
}

int StreamDockN1::setBackgroundImg(const std::string& path)
{
	(void)path;
	return 0;
}

int StreamDockN1::setBackgroundImgData(unsigned char* imagedata, RgbaFormat pitch, unsigned int loadPicWidth, unsigned int loadPicHeight)
{
	(void)imagedata;
	(void)pitch;
	(void)loadPicWidth;
	(void)loadPicHeight;
	return 0;
}

int StreamDockN1::setKeyImg(const std::string& path, int key)
{
	try
	{
		fipImage img;
		if (img.load(path.c_str()))
		{
			const std::string tmpFileName = "setKeyImg_tmp.jpg";
			if (isKeySecondScreen(key)) PicHelper::toNativeKeyImage_secondScreen(img, this);
			else if (isKeyNormal(key))  PicHelper::toNativeKeyImage(img, this);
			else return 0;
			int size = ky_width * ky_height * 3;
			img.save(FIF_JPEG, tmpFileName.c_str(), 90);
			auto ret = this->transport->setKeyImgDualDevice(tmpFileName.c_str(), key);
			std::remove(tmpFileName.c_str());
			return ret;
		}
		else
			throw std::runtime_error("Failed to load image from: " + path);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error in setKeyImg: " << e.what() << std::endl;
		return 0;
	}
}

int StreamDockN1::setKeyImgData(unsigned char* imagedata, int key, RgbaFormat pitch, unsigned int loadPicWidth, unsigned int loadPicHeight)
{
	try
	{
		if (!imagedata)
			throw std::invalid_argument("imagedata is null.");
		if (loadPicWidth == 0 || loadPicHeight == 0)
			throw std::invalid_argument("Invalid image dimensions: width or height is zero.");
		if (static_cast<int>(pitch) <= 0)
			throw std::invalid_argument("Invalid pitch value.");

		FIBITMAP* bitmap = FreeImage_ConvertFromRawBits(imagedata, loadPicWidth, loadPicHeight, loadPicWidth * static_cast<int>(pitch), static_cast<int>(pitch) * 8, 0xFF0000, 0x00FF00, 0x0000FF, false);
		fipImage img;
		img = bitmap; // manage bitmap, can't deconstruct
		if (isKeySecondScreen(key)) PicHelper::toNativeKeyImage_secondScreen(img, this);
		else if (isKeyNormal(key))  PicHelper::toNativeKeyImage(img, this);
		else return 0;
		const std::string tmpFileName = "setKeyImgData_tmp.jpg";
		img.save(FIF_JPEG, tmpFileName.c_str(), 90);
		auto ret = this->transport->setKeyImgDataDualDevice(tmpFileName, key);
		std::remove(tmpFileName.c_str());
		return ret;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error in setKeyImgData: " << e.what() << std::endl;
		return 0;
	}
}

int StreamDockN1::switchMode(N1MODE mode)
{
	return this->transport->switchMode(mode);
}
