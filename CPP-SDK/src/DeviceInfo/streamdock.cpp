#include "streamdock.h"
#include <fstream>
#include <iostream>
#include <mutex>
#include <Gif2ImgFrame.h>
#include <toolkit.h>

StreamDock::StreamDock(const hid_device_info& device_info)
	: _transport(std::move(std::make_unique<TransportCWrapper>(device_info)))
{
	_info = std::make_unique<StreamDockInfo>();
	_feature = std::make_unique<FeatureOption>();
}

StreamDock::~StreamDock()
{
	_readController.reset();
	_gifController.reset();
	_rgbController.reset();
	_heartBeater.reset();
	_transport.reset(); /// 这玩意需要放最后面, 先析构可能会导致前面的访问空指针
}

void StreamDock::init()
{
	_readController = std::make_unique<ReadController>(this);
	_heartBeater = std::make_unique<HeartBeat>(this);
	if (_feature->hasRGBLed)
		_rgbController = std::make_unique<RGBController>(this);
	if (_feature->isDualDevice)
		_gifController = std::make_unique<GifController>(this);
	if (_feature->supportConfig)
		_configer = std::make_unique<Configer>(this);
}

void StreamDock::initImgHelper()
{
	if (!_bg_imgHelper)
		_bg_imgHelper = std::make_shared<ImgHelper>(_info->width, _info->height, _info->bg_rotate_angle, ResizeOption::Scale, _info->bg_flip_vertical, _info->bg_flip_horizonal, _info->backgroundEncodeType, _info->backgroundEncodeFormat);
	if (!_ky_imgHelper)
		_ky_imgHelper = std::make_shared<ImgHelper>(_info->keyWidth, _info->keyHeight, _info->key_rotate_angle, ResizeOption::Scale, _info->key_flip_vertical, _info->key_flip_horizonal, _info->keyEncodeType);
	if (!_2rdsc_imgHelper && _feature->hasSecondScreen)
		_2rdsc_imgHelper = std::make_shared<ImgHelper>(_feature->_2rdScreenWidth, _feature->_2rdScreenHeights, _info->key_rotate_angle, ResizeOption::Scale, _info->key_flip_vertical, _info->key_flip_horizonal, _feature->_2rdScreenEncodeType);
	if (!_bg_gifHelper && _feature->supportBackGroundGif)
		_bg_gifHelper = std::make_shared<ImgHelper>(0, 0, _info->width, _info->height, _info->bg_rotate_angle, ResizeOption::Scale, _info->bg_flip_vertical, _info->bg_flip_horizonal, _feature->backgroundGifEncodeType);
}

std::string StreamDock::getFirmwareVersion()
{
	if (_transport && _info->firmwareVersion.empty())
		return _transport->getFirmwareVesion();
	return _info->firmwareVersion;
}

void StreamDock::wakeupScreen()
{
	if (_transport->canWrite())
		_transport->wakeupScreen();
}
void StreamDock::setKeyBrightness(uint8_t brightness)
{
	if (_transport->canWrite())
		_transport->setKeyBrightness(brightness);
}

void StreamDock::clearAllKeys()
{
	if (_transport->canWrite())
		_transport->clearAllKeys();
}

void StreamDock::clearKey(uint8_t keyValue)
{
	if (outOfRange(keyValue))
	{
		ToolKit::print("[ERROR] Key value out of range: ", static_cast<int>(keyValue));
		return;
	}
	if (_transport->canWrite())
		_transport->clearKey(keyValue);
}

void StreamDock::refresh()
{
	if (_transport->canWrite())
		_transport->refresh();
}

void StreamDock::sleep()
{
	if (_transport->canWrite())
		_transport->sleep();
}

void StreamDock::disconnected()
{
	if (_transport && _transport->canWrite())
		_transport->disconnected();
}

void StreamDock::heartbeat()
{
	if (_transport && _transport->canWrite())
		_transport->heartbeat();
}

void StreamDock::setKeyImgFile(const std::string& filePath, uint8_t keyValue)
{
	if (outOfRange(keyValue))
	{
		ToolKit::print("[ERROR] Key value out of range: ", static_cast<int>(keyValue));
		return;
	}
	if (!_transport || !_transport->canWrite())
	{
		ToolKit::print("[ERROR] Invalid handle or can't write");
		return;
	}
	if (!_encoder)
	{
		ToolKit::print("[ERROR] Encoder is not set, cannot encode image.");
		return;
	}
	auto imgData = readImgToString(filePath);
	std::vector<uint8_t> imgDataVec(imgData.data(), imgData.data() + imgData.size());
	std::vector<uint8_t> output;
	if (_encoder->encodeToMemory(output, imgDataVec, 95, *getKyImgHelper(keyValue)))
		setKeyImgFileStream(std::string(output.data(), output.data() + output.size()), keyValue);
}

void StreamDock::setKeyImgFileStream(const std::string& stream, uint8_t keyValue)
{
	///  we strongly suggest you do not use this directly when it will Invoke `_transport->setKeyBitmap`.
	/// You'd use `StreamDock::setKeyImgFile`
	if (outOfRange(keyValue))
	{
		ToolKit::print("[ERROR] Key value out of range: ", static_cast<int>(keyValue));
		return;
	}
	if (!canTransportWrite())
	{
		ToolKit::print("[ERROR] Encoder is not set, cannot encode image.");
		return;
	}
	if ((_ky_imgHelper->_imgType == ImgType::JPG && !isJpegData(stream)) || (_feature->supportTransparentIcon && _ky_imgHelper->_imgType == ImgType::PNG && !isPngData(stream)))
	{
		ToolKit::print("[ERROR] Invalid Image data, can not match the imgHelper type.");
		return;
	}
	_transport->setKeyImgFileStream(stream, keyValue);
}

void StreamDock::setBackgroundImgFile(const std::string& filePath, uint32_t timeoutMs)
{
	if (!_transport->canWrite())
	{
		ToolKit::print("[ERROR] Invalid handle or can't write");
		return;
	}
	if (!_encoder)
	{
		ToolKit::print("[ERROR] Encoder is not set, cannot encode image.");
		return;
	}
	auto imgData = readImgToString(filePath);
	std::vector<uint8_t> imgDataVec(imgData.data(), imgData.data() + imgData.size());
	std::vector<uint8_t> output;
	if (_encoder->encodeToMemory(output, imgDataVec, 85, *_bg_imgHelper))
		setBackgroundImgStream(std::string(output.data(), output.data() + output.size()), timeoutMs);
}

void StreamDock::setBackgroundImgStream(const std::string& stream, uint32_t timeoutMs)
{
	if (!canTransportWrite())
	{
		ToolKit::print("[ERROR] Transport is not running.");
		return;
	}

	if (_feature->isDualDevice)
	{
		if (!isJpegData(stream))
		{
			ToolKit::print("[ERROR] Invalid JPEG data.");
			return;
		}
		_transport->setBackgroundImgStream(stream, timeoutMs);
	}
	else
	{
		_transport->setBackgroundBitmap(stream, timeoutMs);
	}
}

bool StreamDock::canTransportWrite()
{
	return _transport && _transport->canWrite();
}

bool StreamDock::outOfRange(uint16_t keyValue) const
{
	if (!_info || !_feature)
		return false;
	return !(_info->minKey <= keyValue && keyValue <= _info->maxKey) && (_feature->isDualDevice ? !(_feature->min2rdScreenKey <= keyValue && keyValue <= _feature->max2rdScreenKey) : false);
}

std::wstring StreamDock::lastError()
{
	if (!_transport)
		return L"";
	std::wstring _err;
	_err.resize(128);
	size_t length = 0;
	_transport->rawHidLastError(reinterpret_cast<wchar_t*>(_err.data()), &length);
	_err.resize(length);
	return _err;
}

void StreamDock::disableOutput(bool disable)
{
	TransportCWrapper::disableOutput(disable);
}

StreamDockInfo* StreamDock::info()
{
	return _info.get();
}

FeatureOption* StreamDock::feature()
{
	return _feature.get();
}

IReadController* StreamDock::reader()
{
	if (!_readController)
		_readController = std::make_unique<NullReadController>();
	return _readController.get();
}

IRGBController* StreamDock::rgber()
{
	if (!_rgbController)
		_rgbController = std::make_unique<NullRGBController>();
	return _rgbController.get();
}

IGifController* StreamDock::gifer()
{
	if (!_gifController)
		_gifController = std::make_unique<NullGifController>();
	return _gifController.get();
}

IConfiger* StreamDock::configer()
{
	if (!_configer)
		_configer = std::make_unique<NullConfiger>();
	return _configer.get();
}

IHeartBeat* StreamDock::heartbeater()
{
	if (!_heartBeater)
		_heartBeater = std::make_unique<NullHeartBeat>();
	return _heartBeater.get();
}

std::string StreamDock::readImgToString(const std::string& jpgPath)
{
	std::ifstream ifs(jpgPath, std::ios::binary | std::ios::ate);
	if (!ifs)
		throw std::runtime_error("[ERROR] Failed to open file: " + jpgPath);

	std::streamsize size = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	std::string buffer(size, '\0');
	if (!ifs.read(&buffer[0], size))
		throw std::runtime_error("[ERROR] Failed to read file: " + jpgPath);

	return buffer;
}

bool StreamDock::isStreamDockHidDevice(const hid_device_info& device)
{
	const bool defaultRet = false;

	auto contains = [](const std::wstring& haystack, const std::wstring& needle) -> bool
		{
			return haystack.find(needle) != std::wstring::npos;
		};

	std::wstring vendor = device.manufacturer_string;
	std::wstring product = device.product_string;
	if (contains(vendor, HOTSPOT_STRING) || (contains(product, HOTSPOT_STRING) && contains(product, HOTSPOT_HID_STRING)))
	{
		return true;
	}

	return defaultRet;
}

bool StreamDock::isStreamDockHidDeviceUsage(const hid_device_info& device)
{
	const bool defaultRet = false;
	auto usage = device.usage;
	auto usage_page = device.usage_page;
	auto interface_number = device.interface_number;
	if (usage == 1 && usage_page == 65440)
	{
		return true;
	}
	return false;
}

bool StreamDock::isJpegData(const std::string& originData)
{
	if (!USE_JPEG_STRICT)
		return true;

	if (originData.size() < 4)
		return false;

	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(originData.data());
	size_t len = originData.size();

	if (bytes[0] != 0xFF || bytes[1] != 0xD8)
		return false;

	// EOI marker (optional but recommended)
	if (bytes[len - 2] == 0xFF && bytes[len - 1] == 0xD9)
		return true;

	return true; // truncated JPEG still considered valid (e.g. MJPEG)
}

bool StreamDock::isPngData(const std::string& originData)
{
	if (!USE_PNG_STRICT)
		return true;

	if (originData.size() < 8)
		return false;

	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(originData.data());

	// PNG header signature: 89 50 4E 47 0D 0A 1A 0A
	return bytes[0] == 0x89 &&
		bytes[1] == 0x50 &&
		bytes[2] == 0x4E &&
		bytes[3] == 0x47 &&
		bytes[4] == 0x0D &&
		bytes[5] == 0x0A &&
		bytes[6] == 0x1A &&
		bytes[7] == 0x0A;
}

std::vector<std::vector<uint8_t>> StreamDock::readGifToStream(const std::string& filePath, std::shared_ptr<IImageEncoder> encoder, const ImgHelper& helper)
{
	if (!encoder || helper == ImgHelper())
	{
		ToolKit::print("[ERROR] This Encoder or ImgHelper is not set, cannot encode GIF.");
		return {};
	}
	Gif2ImgFrame gif(filePath, encoder);
	if (!gif.isValid())
	{
		ToolKit::print("[ERROR] failed to load gif");
		return {};
	}
	// 优化：降低JPEG质量以减少USB传输时间，提升流畅度
	// 质量70在画质和传输速度之间取得良好平衡
	int quality = 70;
#if __linux__
	quality = 60;   /// if you use linux in virtual mechine, you should not send high quality pic to device
#endif
	return gif.encodeFramesToMemory(quality, helper);
}

std::vector<GifFrameData> StreamDock::readGifWithDelays(const std::string& filePath, std::shared_ptr<IImageEncoder> encoder, const ImgHelper& helper)
{
	if (!encoder || helper == ImgHelper())
	{
		ToolKit::print("[ERROR] This Encoder or ImgHelper is not set, cannot encode GIF.");
		return {};
	}
	Gif2ImgFrame gif(filePath, encoder);
	if (!gif.isValid())
	{
		ToolKit::print("[ERROR] failed to load gif");
		return {};
	}
	// 优化：降低JPEG质量以减少USB传输时间，提升流畅度
	// 质量70在画质和传输速度之间取得良好平衡
	int quality = 70;
#if __linux__
	quality = 60;   /// if you use linux in virtual mechine, you should not send high quality pic to device
#endif
	return gif.encodeFramesWithDelay(quality, helper);
}

void StreamDock::setEncoder(std::shared_ptr<IImageEncoder> encoder)
{
	_encoder = std::move(encoder);
}

std::shared_ptr<ImgHelper> StreamDock::getBgImgHelper() const
{
	static std::shared_ptr<ImgHelper> nullKyImgHelper = std::make_shared<ImgHelper>();
	return _bg_imgHelper ? _bg_imgHelper : nullKyImgHelper;
}

std::shared_ptr<ImgHelper> StreamDock::getKyImgHelper(uint16_t keyValue) const
{
	static std::shared_ptr<ImgHelper> nullKyImgHelper = std::make_shared<ImgHelper>();
	if (_info->minKey <= keyValue && keyValue <= _info->maxKey && _ky_imgHelper)
		return _ky_imgHelper;
	else if (_feature->hasSecondScreen && _feature->min2rdScreenKey <= keyValue && keyValue <= _feature->max2rdScreenKey && _2rdsc_imgHelper)
		return _2rdsc_imgHelper;
	else
		return nullKyImgHelper;
}

std::shared_ptr<ImgHelper> StreamDock::getSecondScreenImgHelper(uint16_t keyValue) const
{
	static std::shared_ptr<ImgHelper> nullKyImgHelper = std::make_shared<ImgHelper>();
	return _2rdsc_imgHelper ? _2rdsc_imgHelper : nullKyImgHelper;
}

std::shared_ptr<ImgHelper> StreamDock::getBackgroundGifHelper(uint16_t keyValue) const
{
	static std::shared_ptr<ImgHelper> nullKyImgHelper = std::make_shared<ImgHelper>();
	return _bg_gifHelper ? _bg_gifHelper : nullKyImgHelper;
}
