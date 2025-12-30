/**
 * @file streamdock.h
 * @brief Abstract base class for StreamDock devices, handling communication, image rendering, and feature modules.
 *
 * The StreamDock class provides core functionality shared by all StreamDock devices, including:
 * - Communication with the hardware via TransportCWrapper
 * - Image rendering (key and background)
 * - Device info, feature flags, and image helpers
 * - Component-based architecture: supports pluggable controllers for input, RGB, GIFs, config, heartbeat, etc.
 * - Event dispatch mechanism via the pure virtual method dispatchEvent()
 *
 * It is designed to be subclassed per device model (e.g., StreamDockM18, StreamDockN4), with specific behaviors
 * implemented in the derived class.
 */
#pragma once
#include "hidapi.h"
#include <TransportCWrapper.h>
#include <streamdockinfo.h>
#include <featureoption.h>
#include <memory>
#include <Feature/ReadController/readcontroller.h>
#include <Feature/RGBController/rgbcontroller.h>
#include <Feature/GifController/gifcontroller.h>
#include <Feature/Configer/configer.h>
#include <Feature/HeartBeat/heartbeat.h>
#include <ImgHelper.h>
#include <unordered_map>
#include <IImageEncoder.h>
#include "Gif2ImgFrame.h"

static constexpr auto HOTSPOT_STRING = L"HOTSPOT";
static constexpr auto HOTSPOT_HID_STRING = L"HID";
static constexpr auto USE_JPEG_STRICT = true;
static constexpr auto USE_PNG_STRICT = true;
static constexpr auto READ_LOOP_TIMEOUT = 100;

class IReadController;
class IRGBController;
class IGifController;
class IConfiger;
class IHeartBeat;
class StreamDock
{
	friend class ReadController;
	friend class RGBController;
	friend class GifController;
	friend class Configer;
	friend class HeartBeat;

public:
	explicit StreamDock(const hid_device_info& device_info);
	virtual ~StreamDock();

protected:
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) = 0;

public:
	/**
	 * @brief Initialize the StreamDock device and all internal components.
	 */
	void init();
	/**
	 * @brief Initialize internal image helpers (e.g., background, key, second screen).
	 */
	void initImgHelper();

	std::string getFirmwareVersion();
	/**
	 * @brief Wake up the device screen.
	 */
	virtual void wakeupScreen();

	/**
	 * @brief Set the brightness of the device keys.
	 * @param brightness Brightness level (typically 0–100).
	 */
	virtual void setKeyBrightness(uint8_t brightness);

	/**
	 * @brief Clear all key images on the device.
	 */
	virtual void clearAllKeys();

	/**
	 * @brief Clear a specific key's image.
	 * @param keyValue Index of the key to clear.
	 */
	virtual void clearKey(uint8_t keyValue);

	/**
	 * @brief Refresh the display.
	 */
	virtual void refresh();

	/**
	 * @brief Put the device into sleep mode.
	 */
	virtual void sleep();

	/**
	 * @brief Disconnect the device or mark it as disconnected.
	 */
	virtual void disconnected();

	/**
	 * @brief Send a heartbeat signal to the device.
	 */
	virtual void heartbeat();

	/**
	 * @brief Set a key image from a file path.
	 * @param filePath Path to a JPEG or PNG(on N4Pro) image file.
	 * @param keyValue Target key index.
	 */
	virtual void setKeyImgFile(const std::string& filePath, uint8_t keyValue);

	/**
	 * @brief Set a key image using raw JPEG/PNG(on N4Pro) data stream.
	 * @param stream Image byte stream.
	 * @param keyValue Target key index.
	 */
	virtual void setKeyImgFileStream(const std::string& stream, uint8_t keyValue);

	/**
	 * @brief Set the full background image from a file.
	 * @param filePath Image file path.
	 * @param timeoutMs Timeout in milliseconds.
	 */
	virtual void setBackgroundImgFile(const std::string& filePath, uint32_t timeoutMs = 3000);

	/**
	 * @brief Set the full background image from jpeg_data(on new firmware) or bitmap_data(on V2 firmware).
	 * @param stream Image data stream.
	 * @param timeoutMs Timeout in milliseconds.
	 */
	virtual void setBackgroundImgStream(const std::string& stream, uint32_t timeoutMs = 3000);

public:
	/**
	 * @brief Check if the transport layer is ready for writing.
	 * @return True if writing is available.
	 */
	bool canTransportWrite();

	/**
	 * @brief Check if the key index is out of the supported range.
	 * @param keyValue Key index to check.
	 * @return True if out of range.
	 */
	bool outOfRange(uint16_t keyValue) const;

	/**
	 * @brief Get the last error message from the device.
	 * @return Last error message in wide string.
	 */
	std::wstring lastError();

	/**
	 * @brief Enable or disable transport global output (e.g., debug logging).
	 * @param disable Whether to disable output.
	 */
	static void disableOutput(bool disable);

public:
	/**
	 * @brief Get device metadata.
	 * @return Pointer to StreamDockInfo.
	 */
	StreamDockInfo* info();

	/**
	 * @brief Get supported feature options.
	 * @return Pointer to FeatureOption structure.
	 */
	FeatureOption* feature();

public:
	/**
	 * @brief Get the input reader controller.
	 */
	IReadController* reader();

	/**
	 * @brief Get the RGB controller.
	 */
	IRGBController* rgber();

	/**
	 * @brief Get the GIF animation controller.
	 */
	IGifController* gifer();

	/**
	 * @brief Get the configuration controller.
	 */
	IConfiger* configer();

	/**
	 * @brief Get the heartbeat controller.
	 */
	IHeartBeat* heartbeater();

public:
	/**
	 * @brief Read an image file and return it as a byte stream.
	 * @param jpgPath Path to a JPEG image.
	 * @return Byte string of image data.
	 */
	static std::string readImgToString(const std::string& jpgPath);

	/**
	 * @brief Check if the device is a valid StreamDock HID device.
	 */
	static bool isStreamDockHidDevice(const hid_device_info& device);

	/**
	 * @brief Check if the HID usage of the device matches StreamDock.
	 */
	static bool isStreamDockHidDeviceUsage(const hid_device_info& device);

	/**
	 * @brief Check if the given data is JPEG encoded.
	 */
	static bool isJpegData(const std::string& originData);

	/**
	 * @brief Check if the given data is PNG encoded.
	 */
	static bool isPngData(const std::string& originData);

	/**
	 * @brief Read a GIF file and split it into encoded image frames.
	 * @param filePath Path to the GIF file.
	 * @param encoder Image encoder.
	 * @param helper Image helper for formatting.
	 * @return Vector of image frame byte arrays.
	 */
	static std::vector<std::vector<uint8_t>> readGifToStream(const std::string& filePath, std::shared_ptr<IImageEncoder> encoder, const ImgHelper& helper);

	/**
	 * @brief Read a GIF file and split it into encoded image frames with delay times.
	 * @param filePath Path to the GIF file.
	 * @param encoder Image encoder.
	 * @param helper Image helper for formatting.
	 * @return Vector of GifFrameData containing encoded frames and delay times.
	 */
	static std::vector<GifFrameData> readGifWithDelays(const std::string& filePath, std::shared_ptr<IImageEncoder> encoder, const ImgHelper& helper);

public:
	/**
	 * @brief Set the image encoder used for output formatting.
	 */
	void setEncoder(std::shared_ptr<IImageEncoder> encoder);

	/**
	 * @brief Get the helper for background image operations.
	 */
	std::shared_ptr<ImgHelper> getBgImgHelper() const;

	/**
	 * @brief Get the helper for key image operations.
	 * @param keyValue Optional key index.
	 */
	std::shared_ptr<ImgHelper> getKyImgHelper(uint16_t keyValue = 0) const;

	/**
	 * @brief Get the helper for second screen image operations.
	 */
	std::shared_ptr<ImgHelper> getSecondScreenImgHelper(uint16_t keyValue = 0) const;

	/**
	 * @brief Get the helper for background GIF animations.
	 */
	std::shared_ptr<ImgHelper> getBackgroundGifHelper(uint16_t keyValue = 0) const;

protected:
	std::unordered_map<uint8_t, uint8_t> _readValueMap;       ///< Key mapping table: maps raw read values (e.g., response[9]) to logical key codes registered by the derived class.

	std::unique_ptr<TransportCWrapper> _transport = nullptr;  ///< Communication handler (transport layer wrapper).
	std::unique_ptr<StreamDockInfo> _info = nullptr;          ///< Device information.
	std::unique_ptr<FeatureOption> _feature = nullptr;        ///< Device feature flags and capabilities.

	std::unique_ptr<IReadController> _readController = nullptr; ///< Input read controller.
	std::unique_ptr<IRGBController> _rgbController = nullptr;   ///< RGB lighting controller.
	std::unique_ptr<IGifController> _gifController = nullptr;   ///< GIF animation controller.
	std::unique_ptr<IConfiger> _configer = nullptr;             ///< Device configuration controller.
	std::unique_ptr<IHeartBeat> _heartBeater = nullptr;         ///< Heartbeat signaling controller.

	std::shared_ptr<IImageEncoder> _encoder = nullptr;          ///< Image encoder used for output formatting.
	std::shared_ptr<ImgHelper> _bg_imgHelper = nullptr;         ///< Background image helper.
	std::shared_ptr<ImgHelper> _ky_imgHelper = nullptr;         ///< Key image helper.
	std::shared_ptr<ImgHelper> _2rdsc_imgHelper = nullptr;      ///< Second screen image helper.
	std::shared_ptr<ImgHelper> _bg_gifHelper = nullptr;         ///< Background GIF animation helper.

};