#include "streamdockN1.h"
#include <iostream>
#include <opencv2/opencv.hpp>

static constexpr auto VID_STREAMDOCK_N1 = 0x6603;
static constexpr auto PID_STREAMDOCK_N1 = 0x1011;

static constexpr auto VID_STREAMDOCK_N1E = 0x6603;
static constexpr auto PID_STREAMDOCK_N1E = 0x1000;

bool StreamDockN1::registered_N1 = []()
{
	StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_N1, PID_STREAMDOCK_N1,
												 [](const hid_device_info &device_info)
												 {
													 auto device = std::make_unique<StreamDockN1>(device_info);
													 device->init();
													 device->initImgHelper();
													 return device;
												 });
	return true;
}();

bool StreamDockN1::registered_N1E = []()
{
	StreamDockFactory::instance().registerDevice(VID_STREAMDOCK_N1E, PID_STREAMDOCK_N1E,
												 [](const hid_device_info &device_info)
												 {
													 auto device = std::make_unique<StreamDockN1>(device_info);
													 device->init();
													 device->initImgHelper();
													 return device;
												 });
	return true;
}();

StreamDockN1::StreamDockN1(const hid_device_info &device_info)
	: StreamDock(device_info)
{
	_transport->setReportSize(513, 1025, 0);
	_info->originType = DeviceOriginType::SDN1;
	_info->vendor_id = device_info.vendor_id;
	_info->product_id = device_info.product_id;
	_info->serialNumber = device_info.serial_number;
	std::wcout << "StreamDockN1: " << _info->serialNumber << std::endl;
	_info->keyWidth = 96;
	_info->keyHeight = 96;
	_info->minKey = 1;
	_info->maxKey = 15;
	_info->width = 480;
	_info->height = 854;
	_info->key_rotate_angle = 0.0f;
	_info->bg_rotate_angle = 0.0f;
	_feature->isDualDevice = true;
	_feature->hasSecondScreen = true;
	_feature->min2rdScreenKey = 16;
	_feature->max2rdScreenKey = 18;
	_feature->_2rdScreenWidth = 64;
	_feature->_2rdScreenHeights = 64;
	// clang-format off
	_readValueMap = {
		/// Normal keys, starting from the top-left corner, counted left to right and top to bottom, correspond to keys 1 to 15
		{1, 0x01}, {2, 0x02}, {3, 0x03}, {4, 0x04}, {5, 0x05},
		{6, 0x06},{7, 0x07},{8, 0x08},{9, 0x09},{10, 0x0A},
		{11, 0x0B},{12, 0x0C},{13, 0x0D},{14, 0x0E},{15, 0x0F},
		/// The two top buttons
		{16, 0x1E},{17, 0x1F},
		/// Knob press
		{18, 0x23},
		/// Knob rotate left and right
		{19, 0x32},{20, 0x33}
	};
	// clang-format on
}

RegisterEvent StreamDockN1::dispatchEvent(uint8_t readValue, uint8_t eventValue)
{
	if ((0x01 <= readValue && readValue <= 0x0F) && eventValue == 0x00)
		return RegisterEvent::KeyRelease; /// Normal key release event
	else if ((0x01 <= readValue && readValue <= 0x0F) && eventValue == 0x01)
		return RegisterEvent::KeyPress; /// Normal key press event
	if ((0x1E == readValue || readValue == 0x1F) && eventValue == 0x00)
		return RegisterEvent::KeyRelease; /// Top button release event
	else if ((0x1E == readValue || readValue == 0x1F) && eventValue == 0x01)
		return RegisterEvent::KeyPress; /// Top button press event
	if (0x23 == readValue && eventValue == 0x00)
		return RegisterEvent::KnobRelease; /// Knob release event
	else if (0x23 == readValue && eventValue == 0x01)
		return RegisterEvent::KnobPress; /// Knob press event
	if (0x32 == readValue && eventValue == 0x00)
		return RegisterEvent::KnobLeft; /// Knob rotate left event
	else if (0x33 == readValue && eventValue == 0x00)
		return RegisterEvent::KnobRight; /// Knob rotate right event
	return RegisterEvent::EveryThing;
}

void StreamDockN1::changeMode(N1MODE mode)
{
	if (!_transport)
		return;
	_transport->changeMode(static_cast<uint8_t>(mode));
}

void StreamDockN1::changePage(uint8_t page)
{
	if (!_transport)
		return;
	_transport->changePage(page);
}

void StreamDockN1::setSkinBitmap(const std::string &bitmap_path, SkinMode skin_mode, uint8_t skin_page, SkinStatus skin_status, uint8_t key_index, int32_t timeout_ms)
{
	if (!_transport)
		return;
	if (SkinMode::CALCULATOR == skin_mode)
	{
		if (key_index < 1 || key_index > 18)
		{
			std::cerr << "Error: For CALCULATOR skin mode, key_index should be in range 1-18." << std::endl;
			return;
		}
	}
	else if (SkinMode::KEYBOARD == skin_mode || SkinMode::KEYBOARD_LOCK == skin_mode)
	{
		if (key_index < 1 || key_index > 15)
		{
			std::cerr << "Error: For KEYBOARD skin mode, key_index should be in range 1-15." << std::endl;
			return;
		}
	}
	if (skin_page < 1 || skin_page > 5)
	{
		std::cerr << "Error: skin_page should be in range 1-5." << std::endl;
		return;
	}
	// if bitmap is not png, Convert file format to png
	cv::Mat img = cv::imread(bitmap_path, cv::IMREAD_UNCHANGED);
	if (img.empty())
	{
		std::cerr << "Error: Could not load image at " << bitmap_path << std::endl;
		return;
	}
	if (SkinMode::KEYBOARD == skin_mode && key_index >= 16 && key_index <= 18)
		cv::resize(img, img, cv::Size(64, 64));
	else
		cv::resize(img, img, cv::Size(96, 96));
	std::string final_bitmap_path;
	if (bitmap_path.substr(bitmap_path.find_last_of(".") + 1) != "png")
	{
		std::vector<uchar> buf;
		cv::imencode(".png", img, buf);
		final_bitmap_path = "temp_skin.png";
		cv::imwrite(final_bitmap_path, img);
	}
	else
	{
		final_bitmap_path = bitmap_path;
	}
	// read bitmap file and send to device
	std::string final_bitmap;
	{
		std::ifstream file(final_bitmap_path, std::ios::binary);
		if (!file)
		{
			std::cerr << "Error: Could not open image file " << final_bitmap_path << std::endl;
			return;
		}
		std::ostringstream ss;
		ss << file.rdbuf();
		final_bitmap = ss.str();
	}
	_transport->setN1SkinBitmap(final_bitmap, static_cast<uint8_t>(skin_mode), skin_page, static_cast<uint8_t>(skin_status), key_index, timeout_ms);
	// delete the temporary png file if it was created
	if (final_bitmap_path == "temp_skin.png")
	{
		if (std::remove(final_bitmap_path.c_str()) != 0)
		{
			std::cerr << "Warning: Could not delete temporary file " << final_bitmap_path << std::endl;
		}
	}
}

int extract_last_number(const std::string &code)
{
	size_t last_dot = code.rfind('.');
	if (last_dot == std::string::npos)
	{
		return -1;
	}

	size_t start = last_dot + 1;
	std::string num_str;
	while (start < code.length() && std::isdigit(code[start]))
	{
		num_str += code[start];
		start++;
	}
	if (num_str.empty())
	{
		return -1;
	}

	return std::stoi(num_str);
}

void StreamDockN1::setBackgroundImgFile(const std::string &filePath, uint32_t timeoutMs)
{
	if (extract_last_number(info()->firmwareVersion) >= 13)
		StreamDock::setBackgroundImgFile(filePath, timeoutMs);
}
void StreamDockN1::setBackgroundImgStream(const std::string &stream, uint32_t timeoutMs)
{
	if (extract_last_number(info()->firmwareVersion) >= 13)
		StreamDock::setBackgroundImgStream(stream, timeoutMs);
}