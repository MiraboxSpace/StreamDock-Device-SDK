#include "StreamDock.h"
#include <sstream>
#include <cstring>

StreamDock::StreamDock(const std::shared_ptr<TranSport> &transport, struct hid_device_info *devInfo) : info(devInfo)
{
	this->transport = transport;
}

StreamDock::~StreamDock()
{
}

std::vector<unsigned char> StreamDock::getFirmVersion(int length)
{
	std::vector<unsigned char> report;
	report.resize(length);
	if (this->transport->getInputReport(report.data(), length) == -1)
	{
		return std::vector<unsigned char>();
	}
	return report;
}

int StreamDock::open(bool auto_read)
{
	return this->transport->open(this->info.path.c_str(), auto_read);
}

void StreamDock::init(int brightness)
{
	this->transport->wakeScreen();
	this->transport->setBrightness(brightness);
	this->transport->keyAllClear();
	this->transport->refresh();
}

int StreamDock::disconnected()
{
	return this->transport->disconnected();
}

int StreamDock::setBrightness(int percent)
{
	return this->transport->setBrightness(percent);
}

int StreamDock::refresh()
{
	return this->transport->refresh();
}

std::string StreamDock::getPath()
{
	return this->info.path;
}

int StreamDock::wakeScreen()
{
	return this->transport->wakeScreen();
}

int StreamDock::clearIcon(int index)
{
	return this->transport->keyClear(index);
}

int StreamDock::clearAllIcon()
{
	return this->transport->keyAllClear();
}

int StreamDock::switchMode(N1MODE mode)
{
	// do nothing
	return 0;
}

void StreamDock::setKeyFormat(unsigned int width, unsigned int height)
{
	ky_width = width;
	ky_height = height;
}

void StreamDock::setKeyFormat_secondScreen(unsigned int width, unsigned int height)
{
	ky_width_secondScreen = width;
	ky_height_secondScreen = height;
}

void StreamDock::setBgiFormat(unsigned int width, unsigned int height)
{
	bg_width = width;
	bg_height = height;
}

void StreamDock::setRotate(float kyAngle, float bgAngle)
{
	kyRotateAngle = kyAngle;
	bgRotateAngle = bgAngle;
}

void StreamDock::setFlipHorizonal(bool bgFlip, bool keyFlip)
{
	flipBgHorizonal = bgFlip;
	flipKyHorizonal = keyFlip;
}

void StreamDock::setFlipVertical(bool bgFlip, bool keyFlip)
{
	flipBgVertical = bgFlip;
	flipKyVertical = keyFlip;
}

void StreamDock::setKeyRange(unsigned int minKeyNum, unsigned int maxKeyNum)
{
	maxKey = maxKeyNum;
	minKey = minKeyNum;
}

void StreamDock::setKeySecondScreenRange(unsigned int minKeyNum, unsigned int maxKeyNum)
{
	maxSecondKey = maxKeyNum;
	minSecondKey = minKeyNum;
}

bool StreamDock::isKeyNormal(unsigned int key)
{
	return minKey <= key && key <= maxKey;
}

bool StreamDock::isKeySecondScreen(unsigned int key)
{
	return minSecondKey <= key && key <= maxSecondKey;
}

std::vector<unsigned char> StreamDock::read()
{
	std::vector<unsigned char> buffer;
	buffer.resize(13);
	if (this->transport->read(buffer.data(), 13) == -1)
	{
		return std::vector<unsigned char>();
	}
	return buffer;
}

std::string StreamDock::parseRead(const std::vector<unsigned char>& readVec)
{
	if (readVec.size() != 13) return "";
	std::stringstream ss;
	if (strncmp("OK", reinterpret_cast<const char*>(readVec.data()) + 5, 2) == 0)
	{
		for (int i = 0; i < 5; ++i) ss << readVec.data()[i];
		ss << " ";
		ss << static_cast<char>(readVec.data()[5]);
		ss << static_cast<char>(readVec.data()[6]);
		ss << " ";
		ss << static_cast<int>(readVec.data()[9]);
		ss << " ";
		ss << static_cast<int>(readVec.data()[10]);
	}
	return ss.str();
}

void PicHelper::toNativeBackground(fipImage &img, StreamDock *dock)
{
	// conver to rgb
	img.convertTo24Bits();
	// resize
	img.rescale(dock->bg_width, dock->bg_height, FILTER_BILINEAR);
	// rotate
	if (dock->bgRotateAngle != 0.0f)
	{
		img.rotate(dock->bgRotateAngle);
	}
	// flip
	if (dock->flipBgHorizonal)
	{
		img.flipHorizontal();
	}
	if (dock->flipBgVertical)
	{
		img.flipVertical();
	}
}

void PicHelper::toNativeKeyImage(fipImage &img, StreamDock *dock)
{
	// conver to rgb
	img.convertTo24Bits();
	// resize
	img.rescale(dock->ky_width, dock->ky_height, FILTER_BILINEAR);
	// rotate
	if (dock->kyRotateAngle != 0.0f)
	{
		img.rotate(dock->kyRotateAngle);
	}
	// flip
	if (dock->flipKyHorizonal)
	{
		img.flipHorizontal();
	}
	if (dock->flipKyVertical)
	{
		img.flipVertical();
	}
}

void PicHelper::toNativeKeyImage_secondScreen(fipImage& img, StreamDock* dock)
{
	// conver to rgb
	img.convertTo24Bits();
	// resize
	img.rescale(dock->ky_width_secondScreen, dock->ky_height_secondScreen, FILTER_BILINEAR);
	// rotate
	if (dock->kyRotateAngle != 0.0f)
	{
		img.rotate(dock->kyRotateAngle);
	}
	// flip
	if (dock->flipKyHorizonal)
	{
		img.flipHorizontal();
	}
	if (dock->flipKyVertical)
	{
		img.flipVertical();
	}
}