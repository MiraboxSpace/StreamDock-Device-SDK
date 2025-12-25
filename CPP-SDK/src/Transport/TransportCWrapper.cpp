#include "TransportCWrapper.h"
#include <stdexcept>
#include <iostream>

TransportCWrapper::TransportCWrapper(const hid_device_info& device_info)
{
	transport_create(&device_info, &_handle);
}

TransportCWrapper::~TransportCWrapper()
{
	if (_handle)
	{
		transport_destroy(_handle);
		_handle = nullptr;
	}
}

TransportCWrapper::TransportCWrapper(TransportCWrapper&& other) noexcept
	: _handle(other._handle)
{
	other._handle = nullptr;
}

TransportCWrapper& TransportCWrapper::operator=(TransportCWrapper&& other) noexcept
{
	if (this != &other)
	{
		if (_handle)
			transport_destroy(_handle);
		_handle = other._handle;
		other._handle = nullptr;
	}
	return *this;
}

std::string TransportCWrapper::getFirmwareVesion() const
{
	if (!_handle) return "";
	std::string firmwareVersion(_input_report_size, '\0');
	transport_get_firmware_version(_handle, firmwareVersion.data(), firmwareVersion.size());
	return firmwareVersion;
}

void TransportCWrapper::clearTaskQueue() const
{
	if (!_handle) return;
	transport_clear_task_queue(_handle);
}

bool TransportCWrapper::canWrite() const
{
	if (!_handle) return false;
	int can_write = 0;
	transport_can_write(_handle, &can_write);
	return can_write != 0;
}

void TransportCWrapper::read(uint8_t* response, size_t* length, int32_t timeoutMs) const
{
	if (!_handle || !response || !length)
		throw std::invalid_argument("Invalid arguments for read operation.");
	transport_read(_handle, response, length, timeoutMs);
}

void TransportCWrapper::wakeupScreen() const
{
	if (!_handle) return;
	transport_wakeup_screen(_handle);
}

void TransportCWrapper::setKeyBrightness(uint8_t brightness) const
{
	if (!_handle) return;
	transport_set_key_brightness(_handle, brightness);
}

void TransportCWrapper::clearAllKeys() const
{
	if (!_handle) return;
	transport_clear_all_keys(_handle);
}

void TransportCWrapper::clearKey(uint8_t key_value) const
{
	if (!_handle) return;
	transport_clear_key(_handle, key_value);
}

void TransportCWrapper::refresh() const
{
	if (!_handle) return;
	transport_refresh(_handle);
}

void TransportCWrapper::sleep() const
{
	if (!_handle) return;
	transport_sleep(_handle);
}

void TransportCWrapper::disconnected() const
{
	if (!_handle) return;
	transport_disconnected(_handle);
}

void TransportCWrapper::heartbeat() const
{
	if (!_handle) return;
	transport_heartbeat(_handle);
}

//void TransportCWrapper::setKeyBitmap(const std::string &bitmapStream, uint8_t keyValue) const
//{
//    if (!_handle) return;
//    transport_set_key_bitmap(_handle, bitmapStream.data(), bitmapStream.size(), keyValue);
//}

void TransportCWrapper::setBackgroundBitmap(const std::string& bitmapStream, int32_t timeoutMs) const
{
	if (!_handle) return;
	transport_set_background_bitmap(_handle, bitmapStream.data(), bitmapStream.size(), timeoutMs);
}

//void TransportCWrapper::setKeyImgFile(const std::string &filePath, uint8_t keyValue) const
//{
//    if (!_handle) return;
//    transport_set_key_image(_handle, filePath.data(), keyValue);
//}

void TransportCWrapper::setKeyImgFileStream(const std::string& jpegData, uint8_t keyValue) const
{
	if (!_handle) return;
	transport_set_key_image_stream(_handle, jpegData.data(), jpegData.size(), keyValue);
}

//void TransportCWrapper::setBackgroundImgFile(const std::string &filePath, int32_t timeoutMs) const
//{
//    if (!_handle) return;
//    transport_set_background_image(_handle, filePath.data(), timeoutMs);
//}

void TransportCWrapper::setBackgroundImgStream(const std::string& jpegData, int32_t timeoutMs) const
{
	if (!_handle) return;
	transport_set_background_image_stream(_handle, jpegData.data(), jpegData.size(), timeoutMs);
}

void TransportCWrapper::setBackgroundFrameStream(const std::string& jpegData, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t FBlayer) const
{
	if (!_handle) return;
	transport_set_background_frame_stream(_handle, jpegData.data(), jpegData.size(), width, height, x, y, FBlayer);
}

void TransportCWrapper::clearBackgroundFrameStream(uint8_t postion) const
{
	if (!_handle) return;
	transport_clear_background_frame_stream(_handle, postion);
}

void TransportCWrapper::setLedBrightness(uint8_t brightness) const
{
	if (!_handle) return;
	transport_set_led_brightness(_handle, brightness);
}

void TransportCWrapper::setLedColor(uint16_t count, uint8_t r, uint8_t g, uint8_t b) const
{
	if (!_handle) return;
	transport_set_led_color(_handle, count, r, g, b);
}

void TransportCWrapper::resetLedColor() const
{
	if (!_handle) return;
	transport_reset_led_color(_handle);
}

void TransportCWrapper::setDeviceConfig(std::vector<uint8_t> configs) const
{
	if (!_handle) return;
	transport_set_device_config(_handle, configs.data(), configs.size());
}

void TransportCWrapper::changeMode(uint8_t mode) const
{
	if (!_handle) return;
	transport_change_mode(_handle, mode);
}

void TransportCWrapper::setReportID(uint8_t reportID) const
{
	if (!_handle) return;
	transport_set_reportID(_handle, reportID);
}

uint8_t TransportCWrapper::reportID() const
{
	if (!_handle) return 0x00;
	uint8_t report_id = 0x00;
	transport_reportID(_handle, &report_id);
	return report_id;
}

void TransportCWrapper::setReportSize(uint16_t input_report_size, uint16_t output_report_size, uint16_t feature_report_size)
{
	if (!_handle) return;
	_input_report_size = input_report_size;
	_output_report_size = output_report_size;
	_feature_report_size = feature_report_size;
	transport_set_reportSize(_handle, input_report_size, output_report_size, feature_report_size);
}

void TransportCWrapper::rawHidLastError(wchar_t* errMsg, size_t* length) const
{
	if (!_handle) return;
	transport_raw_hid_last_error(_handle, errMsg, length);
}

void TransportCWrapper::disableOutput(bool isDisable)
{
	transport_disable_output(static_cast<int8_t>(isDisable));
}

void TransportCWrapper::setKeyboardBacklightBrightness(uint8_t brightness) const
{
	if (!_handle) return;
	transport_set_keyboard_backlight_brightness(_handle, brightness);
}

void TransportCWrapper::setKeyboardLightingEffects(uint8_t effect) const
{
	if (!_handle) return;
	transport_set_keyboard_lighting_effects(_handle, effect);
}

void TransportCWrapper::setKeyboardLightingSpeed(uint8_t speed) const
{
	if (!_handle) return;
	transport_set_keyboard_lighting_speed(_handle, speed);
}

void TransportCWrapper::setKeyboardRgbBacklight(uint8_t red, uint8_t green, uint8_t blue) const
{
	if (!_handle) return;
	transport_set_keyboard_rgb_backlight(_handle, red, green, blue);
}

void TransportCWrapper::keyboardOsModeSwitch(uint8_t os_mode) const
{
	if (!_handle) return;
	transport_keyboard_os_mode_switch(_handle, os_mode);
}
