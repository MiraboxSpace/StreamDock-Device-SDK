#include "streamdockH1Pro.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <random>
#include <system_error>
#include <utility>
#include <vector>
#ifdef _WIN32
#include <process.h>
#else
#include <spawn.h>
#include <sys/wait.h>
extern char** environ;
#endif

namespace
{
constexpr uint16_t VID_STREAMDOCK_H1PRO = 0x5548;
constexpr uint16_t PID_STREAMDOCK_H1PRO = 0x1030;
constexpr uint16_t PID_STREAMDOCK_H1PROE = 0x1033;
namespace fs = std::filesystem;

struct TemporaryDirectory
{
	fs::path path;
	~TemporaryDirectory()
	{
		if (!path.empty())
		{
			std::error_code error;
			fs::remove_all(path, error);
		}
	}
};

bool runFfmpeg(const fs::path& input, const fs::path& output, int rate, int quality)
{
	const auto filter = "fps=" + std::to_string(rate) +
		",transpose=2,scale=240:320:force_original_aspect_ratio=decrease,"
		"pad=240:320:(ow-iw)/2:(oh-ih)/2,format=yuvj420p";
#ifdef _WIN32
	const std::vector<std::wstring> args = {
		L"ffmpeg", L"-hide_banner", L"-loglevel", L"error", L"-y",
		L"-i", input.wstring(), L"-an", L"-vf",
		std::wstring(filter.begin(), filter.end()), L"-c:v", L"mjpeg",
		L"-q:v", std::to_wstring(quality), output.wstring()
	};
	std::vector<const wchar_t*> argv;
	for (const auto& arg : args) argv.push_back(arg.c_str());
	argv.push_back(nullptr);
	return _wspawnvp(_P_WAIT, L"ffmpeg", argv.data()) == 0;
#else
	const std::vector<std::string> args = {
		"ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
		"-i", input.string(), "-an", "-vf", filter, "-c:v", "mjpeg",
		"-q:v", std::to_string(quality), output.string()
	};
	std::vector<char*> argv;
	for (const auto& arg : args) argv.push_back(const_cast<char*>(arg.c_str()));
	argv.push_back(nullptr);
	pid_t child = 0;
	if (posix_spawnp(&child, "ffmpeg", nullptr, nullptr, argv.data(), environ) != 0)
		return false;
	int status = 0;
	return waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0;
#endif
}

std::unique_ptr<StreamDock> createH1Pro(const hid_device_info& info)
{
	auto device = std::make_unique<StreamDockH1Pro>(info);
	device->init();
	device->initImgHelper();
	return device;
}
}

bool StreamDockH1Pro::registered_H1Pro = []()
{
	StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_H1PRO, PID_STREAMDOCK_H1PRO, createH1Pro);
	return true;
}();

bool StreamDockH1Pro::registered_H1ProE = []()
{
	StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_H1PRO, PID_STREAMDOCK_H1PROE, createH1Pro);
	return true;
}();

StreamDockH1Pro::StreamDockH1Pro(const hid_device_info& device_info)
	: StreamDock(device_info)
{
	_transport->setReportSize(513, 1025, 0);
	_info->originType = DeviceOriginType::SDH1Pro;
	_info->vendor_id = device_info.vendor_id;
	_info->product_id = device_info.product_id;
	if (device_info.serial_number)
		_info->serialNumber = device_info.serial_number;
	std::wcout << (device_info.product_id == PID_STREAMDOCK_H1PROE
		? L"StreamDockH1ProE: " : L"StreamDockH1Pro: ")
		<< _info->serialNumber << std::endl;
	_info->width = 320;
	_info->height = 240;
	_info->keyWidth = 64;
	_info->keyHeight = 64;
	_info->minKey = 1;
	_info->maxKey = 12;
	_info->key_rotate_angle = -90.0;
	_info->bg_rotate_angle = -90.0;
	_feature->isDualDevice = true;
	_feature->supportBackGroundGif = false;
	_info->firmwareVersion = getFirmwareVersion();
	std::string firmware = _info->firmwareVersion;
	std::transform(firmware.begin(), firmware.end(), firmware.begin(),
		[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
	// V25/V3 firmware uses 64-pixel keys and JPEG boot logos.
	if (firmware.find("V25") == std::string::npos && firmware.find("V3") == std::string::npos)
	{
		_info->keyWidth = 70;
		_info->keyHeight = 70;
		_info->key_rotate_angle = 0.0;
		_info->bg_rotate_angle = 90.0;
		_info->backgroundEncodeType = ImgType::RAW;
		_info->backgroundEncodeFormat = ImgFormat::RGB16;
	}
	_readValueMap = {
		{1, 0x01}, {2, 0x02}, {3, 0x03}, {4, 0x04},
		{5, 0x05}, {6, 0x06}, {7, 0x07}, {8, 0x08},
		{9, 0x09}, {10, 0x0A}, {11, 0x0B}, {12, 0x0C}
	};
}

RegisterEvent StreamDockH1Pro::dispatchEvent(uint8_t readValue, uint8_t eventValue)
{
	if (readValue >= 0x01 && readValue <= 0x0C)
	{
		if (eventValue == 0x01) return RegisterEvent::KeyPress;
		if (eventValue == 0x00) return RegisterEvent::KeyRelease;
	}
	return RegisterEvent::EveryThing;
}

void StreamDockH1Pro::switchMode(Mode mode)
{
	_transport->changeMode(static_cast<uint8_t>(mode));
}

TransportResult StreamDockH1Pro::uploadMp4Stream(const std::string& mp4Data, uint32_t timeoutMs)
{
	return _transport->uploadH1ProVideo(mp4Data, timeoutMs);
}

TransportResult StreamDockH1Pro::uploadMp4File(const std::string& filePath, uint32_t timeoutMs)
{
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file) return TRANSPORT_ERROR_RESOURCE;
	const std::streamoff size = file.tellg();
	if (size <= 0 || size > 5 * 1024 * 1024) return TRANSPORT_ERROR_PARAM_LENGTH;
	file.seekg(0);
	std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	if (data.size() != static_cast<size_t>(size)) return TRANSPORT_ERROR_RESOURCE;
	return uploadMp4Stream(data, timeoutMs);
}

TransportResult StreamDockH1Pro::uploadGifFile(const std::string& filePath, uint32_t timeoutMs)
{
	std::ifstream gif(filePath, std::ios::binary);
	char signature[6]{};
	if (!gif.read(signature, sizeof(signature)) ||
		(std::string(signature, sizeof(signature)) != "GIF87a" &&
		 std::string(signature, sizeof(signature)) != "GIF89a"))
		return TRANSPORT_ERROR_PARAM_TYPE;
	gif.close();

	TemporaryDirectory temporary;
	std::random_device random;
	std::error_code error;
	for (int attempt = 0; attempt < 16 && temporary.path.empty(); ++attempt)
	{
		auto candidate = fs::temp_directory_path(error) /
			("streamdock_h1pro_" + std::to_string(random()) + "_" + std::to_string(random()));
		if (error) return TRANSPORT_ERROR_RESOURCE;
		if (fs::create_directory(candidate, error)) temporary.path = candidate;
		else if (error && error != std::errc::file_exists) return TRANSPORT_ERROR_RESOURCE;
		error.clear();
	}
	if (temporary.path.empty()) return TRANSPORT_ERROR_RESOURCE;
	const auto output = temporary.path / "animation.mp4";
	const std::pair<int, int> attempts[] = {
		{30, 6}, {20, 10}, {15, 14}, {10, 18}, {8, 20}, {5, 25}, {2, 31}
	};
	TransportResult storageError = TRANSPORT_ERROR_PARAM_LENGTH;
	for (const auto& [rate, quality] : attempts)
	{
		if (!runFfmpeg(fs::path(filePath), output, rate, quality))
			return TRANSPORT_ERROR_RESOURCE;
		const auto size = fs::file_size(output, error);
		if (error) return TRANSPORT_ERROR_RESOURCE;
		if (size == 0 || size > 5U * 1024U * 1024U) continue;
		std::ifstream mp4(output, std::ios::binary);
		if (!mp4) return TRANSPORT_ERROR_RESOURCE;
		std::string data((std::istreambuf_iterator<char>(mp4)), std::istreambuf_iterator<char>());
		if (data.size() != size) return TRANSPORT_ERROR_RESOURCE;
		const auto result = uploadMp4Stream(data, timeoutMs);
		if (result == TRANSPORT_SUCCESS) return result;
		if ((result & 0xFF000000U) != TRANSPORT_ERROR_RESOURCE &&
			_transport->lastErrorMessage().find("exceeds available device storage") == std::string::npos)
			return result;
		storageError = result;
	}
	return storageError;
}
