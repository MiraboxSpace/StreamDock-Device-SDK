/**
 * @file TransportCWrapper.h
 * @brief C++ wrapper class to simplify usage of transport_c.h.
 *
 * Features:
 * - Encapsulates HID device operations: initialization, I/O, image transfer, LED control, etc.
 * - Provides C++-style interfaces for the C-based transport_c.h library
 * - Manages TransportHandle lifetime with RAII
 * - Supports move semantics; copy is disabled
 *
 * Common Interfaces:
 *   - read() / canWrite(): device I/O
 *   - setKeyImgFileStream(), setBackgroundImgStream(): image transmission
 *   - setLedColor(), setLedBrightness(): LED control
 *   - getFirmwareVesion(), changeMode(): device state and control
 *
 * Example (pseudo code):
 *   hid_device_info info = ...;
 *   TransportCWrapper dev(info);
 *   dev.setLedColor(1, 255, 0, 0); // Set LED to red
 *   dev.setBackgroundImgStream(jpegData);
 */

#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "hidapi.h"
#include "./TransportDLL/transport_c.h"

/**
 * @class TransportCWrapper
 * @brief C++ wrapper for transport_c.h to simplify HID device interaction.
 *
 * Provides RAII-style resource management and C++ interface wrapping for various HID operations
 * such as I/O, image transfer, LED control, and configuration.
 * Internally uses TransportHandle to invoke the underlying C functions.
 *
 * - Copy constructor and assignment are disabled to prevent handle duplication.
 * - Move constructor and assignment are supported to allow ownership transfer.
 */
class TransportCWrapper
{
public:
	/**
	 * @brief Initialize connection with a HID device using device info.
	 * @param device_info Device descriptor from hidapi.
	 */
	explicit TransportCWrapper(const hid_device_info& device_info);
	/**
	 * @brief Destructor. Automatically releases underlying TransportHandle.
	 */
	~TransportCWrapper();

	TransportCWrapper(const TransportCWrapper&) = delete;
	TransportCWrapper& operator=(const TransportCWrapper&) = delete;
	/**
	 * @brief Move constructor/assignment. Transfers ownership of internal handle.
	 */
	TransportCWrapper(TransportCWrapper&& other) noexcept;
	TransportCWrapper& operator=(TransportCWrapper&& other) noexcept;

	/**
	 * @brief Get firmware version string from the device.
	 * @return Firmware version as a string.
	 */
	std::string getFirmwareVesion() const;

	/**
     * @brief Clear All the data will be send to device in transport library.
     */
	void clearTaskQueue() const;

	/**
	 * @brief Check if the device is currently writable.
	 */
	bool canWrite() const;

	/**
	 * @brief Read data from the device.
	 * @param response Output buffer to store data.
	 * @param length In: buffer size; Out: actual number of bytes read.
	 * @param timeoutMs Timeout in milliseconds. -1 means blocking read.
	 */
	void read(uint8_t* response, size_t* length, int32_t timeoutMs = -1) const;
	
	/** @brief Wake up the device screen. */
	void wakeupScreen() const;

	/** @brief Set key brightness, usually in range 0-100. */
	void setKeyBrightness(uint8_t brightness) const;

	/** @brief Clear all keys. */
	void clearAllKeys() const;

	/**
	 * @brief Clear the content of a specific key.
	 * @param key_value Index of the key to clear.
	 */
	void clearKey(uint8_t key_value) const;

	/** @brief Refresh screen display. */
	void refresh() const;

	/** @brief Put the device into sleep mode. */
	void sleep() const;

	/** @brief Disconnect the device. */
	void disconnected() const;

	/** @brief Send a heartbeat packet to the device. */
	void heartbeat() const;

	//void setKeyBitmap(const std::string &bitmapStream, uint8_t keyValue) const;

	/**
	 * @brief Set the full-screen background using raw bitmap data.
	 * @param bitmapStream Raw bitmap bytes.
	 * @param timeoutMs Transmission timeout (default 3000ms).
	 */
	void setBackgroundBitmap(const std::string& bitmapStream, int32_t timeoutMs = 5000) const;

	//void setKeyImgFile(const std::string &filePath, uint8_t keyValue) const;

	/**
	 * @brief Set JPEG image to a specific key.
	 * @param jpegData JPEG image data.
	 * @param keyValue Target key index.
	 */
	void setKeyImgFileStream(const std::string& jpegData, uint8_t keyValue) const;

	//void setBackgroundImgFile(const std::string &filePath, int32_t timeoutMs = 3000) const;
	/**
	 * @brief Set JPEG image as full-screen background.
	 * @param jpegData JPEG image data.
	 * @param timeoutMs Transmission timeout.
	 */
	void setBackgroundImgStream(const std::string& jpegData, int32_t timeoutMs = 3000) const;

	/**
	 * @brief Draw a JPEG frame at a specific position (used for animated backgrounds).
	 * @param jpegData JPEG image data.
	 * @param width Image width.
	 * @param height Image height.
	 * @param x X-coordinate.
	 * @param y Y-coordinate.
	 * @param FBlayer Framebuffer layer index.
	 */
	void setBackgroundFrameStream(const std::string& jpegData, uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0, uint8_t FBlayer = 0x00) const;
	
	/**
	 * @brief Clear background frame on the specified framebuffer layer.
	 * @param postion Layer index (default 0x03).
	 */
	void clearBackgroundFrameStream(uint8_t postion = 0x03) const;

	/**
	 * @brief Set LED brightness.
	 * @param brightness Typically ranges from 0 to 100.
	 */
	void setLedBrightness(uint8_t brightness) const;

	/**
	 * @brief Set color for the first N LEDs.
	 * @param count Number of LEDs to set.
	 * @param r Red component.
	 * @param g Green component.
	 * @param b Blue component.
	 */
	void setLedColor(uint16_t count, uint8_t r, uint8_t g, uint8_t b) const;

	/** @brief Reset LED colors. */
	void resetLedColor() const;

	/**
	 * @brief Send raw configuration data to the device.
	 * @param configs Byte array of config values.
	 */
	void setDeviceConfig(std::vector<uint8_t> configs) const;

	/**
	 * @brief Change device working mode.
	 * @param mode Mode identifier.
	 */
	void changeMode(uint8_t mode) const;

	/**
	 * @brief Set the report ID used for communication (default is 0x01).
	 */
	void setReportID(uint8_t reportID) const;

	/**
	 * @brief Get the current report ID.
	 */
	uint8_t reportID() const;

	/**
	 * @brief Set the sizes of the input, output, and feature reports.
	 * @param input_report_size Input report length.
	 * @param output_report_size Output report length.
	 * @param feature_report_size Feature report length.
	 */
	void setReportSize(uint16_t input_report_size, uint16_t output_report_size, uint16_t feature_report_size);

	/**
	 * @brief Get the last raw HID error message.
	 * @param errMsg Output buffer for the error message.
	 * @param length In: buffer size; Out: actual string length written.
	 */
	void rawHidLastError(wchar_t* errMsg, size_t* length) const;

	/**
	 * @brief Globally disable lower-level output (e.g., debug logs).
	 * @param isDisable Whether to disable output (default is true).
	 */
	static void disableOutput(bool isDisable = true);

	/** @brief Set keyboard backlight brightness (K1Pro specific). */
	void setKeyboardBacklightBrightness(uint8_t brightness) const;

	/** @brief Set keyboard lighting effects (K1Pro specific). */
	void setKeyboardLightingEffects(uint8_t effect) const;

	/** @brief Set keyboard lighting speed (K1Pro specific). */
	void setKeyboardLightingSpeed(uint8_t speed) const;

	/** @brief Set keyboard RGB backlight color (K1Pro specific). */
	void setKeyboardRgbBacklight(uint8_t red, uint8_t green, uint8_t blue) const;

	/** @brief Switch keyboard OS mode (K1Pro specific). */
	void keyboardOsModeSwitch(uint8_t os_mode) const;

public:
	uint16_t _input_report_size = 0;   ///< Input report size.
	uint16_t _output_report_size = 0;  ///< Output report size.
	uint16_t _feature_report_size = 0; ///< Feature report size.

private:
	TransportHandle _handle = nullptr; ///< Actual communication handle.
};
