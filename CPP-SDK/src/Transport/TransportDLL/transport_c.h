#ifndef _TRANSPORT_C_H_
#define _TRANSPORT_C_H_
#ifdef _WIN32
#ifdef TRANSPORTDLL_EXPORTS
#define TRANSPORT_API __declspec(dllexport)
#else
#define TRANSPORT_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#define TRANSPORT_API __attribute__((visibility("default")))
#else
#define TRANSPORT_API
#endif
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include "hidapi.h"

	typedef void* TransportHandle;

	// Error code system
	typedef uint32_t TransportResult;

	// Error category (bits 31-24)
	#define TRANSPORT_SUCCESS                    0x00000000U
	#define TRANSPORT_ERROR_PARAM                0x01000000U
	#define TRANSPORT_ERROR_DEVICE               0x02000000U
	#define TRANSPORT_ERROR_MEMORY               0x03000000U
	#define TRANSPORT_ERROR_COMMUNICATION        0x04000000U
	#define TRANSPORT_ERROR_TIMEOUT              0x05000000U
	#define TRANSPORT_ERROR_STATE                0x06000000U
	#define TRANSPORT_ERROR_RESOURCE             0x07000000U
	#define TRANSPORT_ERROR_PROTOCOL             0x08000000U
	#define TRANSPORT_ERROR_UNKNOWN              0xFF000000U

	// Module identifier (bits 15-8)
	#define TRANSPORT_MODULE_CORE                0x00000100U
	#define TRANSPORT_MODULE_DEVICE              0x00000200U
	#define TRANSPORT_MODULE_SESSION             0x00000300U
	#define TRANSPORT_MODULE_BUFFER              0x00000400U

	// Parameter error (0x01xxxxxx)
	#define TRANSPORT_ERROR_PARAM_NULL           (TRANSPORT_ERROR_PARAM | TRANSPORT_MODULE_CORE | 0x01U)
	#define TRANSPORT_ERROR_PARAM_INVALID        (TRANSPORT_ERROR_PARAM | TRANSPORT_MODULE_CORE | 0x02U)
	#define TRANSPORT_ERROR_PARAM_LENGTH         (TRANSPORT_ERROR_PARAM | TRANSPORT_MODULE_CORE | 0x03U)
	#define TRANSPORT_ERROR_PARAM_TYPE           (TRANSPORT_ERROR_PARAM | TRANSPORT_MODULE_CORE | 0x04U)

	// Device error (0x02xxxxxx)
	#define TRANSPORT_ERROR_DEVICE_NOT_CONNECTED (TRANSPORT_ERROR_DEVICE | TRANSPORT_MODULE_DEVICE | 0x01U)
	#define TRANSPORT_ERROR_DEVICE_ACCESS_DENIED (TRANSPORT_ERROR_DEVICE | TRANSPORT_MODULE_DEVICE | 0x02U)
	#define TRANSPORT_ERROR_DEVICE_INVALID_HANDLE (TRANSPORT_ERROR_DEVICE | TRANSPORT_MODULE_DEVICE | 0x03U)
	#define TRANSPORT_ERROR_DEVICE_LOST          (TRANSPORT_ERROR_DEVICE | TRANSPORT_MODULE_DEVICE | 0x04U)
	#define TRANSPORT_ERROR_DEVICE_FAILURE       (TRANSPORT_ERROR_DEVICE | TRANSPORT_MODULE_DEVICE | 0x05U)

	// Memory error (0x03xxxxxx)
	#define TRANSPORT_ERROR_MEMORY_INSUFFICIENT  (TRANSPORT_ERROR_MEMORY | TRANSPORT_MODULE_CORE | 0x01U)
	#define TRANSPORT_ERROR_MEMORY_ALLOC_FAILED  (TRANSPORT_ERROR_MEMORY | TRANSPORT_MODULE_CORE | 0x02U)
	#define TRANSPORT_ERROR_MEMORY_OVERFLOW      (TRANSPORT_ERROR_MEMORY | TRANSPORT_MODULE_CORE | 0x03U)

	// Communication error (0x04xxxxxx)
	#define TRANSPORT_ERROR_COMMUNICATION_WRITE_FAILED (TRANSPORT_ERROR_COMMUNICATION | TRANSPORT_MODULE_DEVICE | 0x01U)
	#define TRANSPORT_ERROR_COMMUNICATION_READ_FAILED  (TRANSPORT_ERROR_COMMUNICATION | TRANSPORT_MODULE_DEVICE | 0x02U)
	#define TRANSPORT_ERROR_COMMUNICATION_CHECKSUM     (TRANSPORT_ERROR_COMMUNICATION | TRANSPORT_MODULE_DEVICE | 0x03U)
	#define TRANSPORT_ERROR_COMMUNICATION_TRUNCATED    (TRANSPORT_ERROR_COMMUNICATION | TRANSPORT_MODULE_DEVICE | 0x04U)

	// Timeout error (0x05xxxxxx)
	#define TRANSPORT_ERROR_TIMEOUT_OPERATION    (TRANSPORT_ERROR_TIMEOUT | TRANSPORT_MODULE_SESSION | 0x01U)
	#define TRANSPORT_ERROR_TIMEOUT_READ        (TRANSPORT_ERROR_TIMEOUT | TRANSPORT_MODULE_SESSION | 0x02U)
	#define TRANSPORT_ERROR_TIMEOUT_WRITE       (TRANSPORT_ERROR_TIMEOUT | TRANSPORT_MODULE_SESSION | 0x03U)

	// State error (0x06xxxxxx)
	#define TRANSPORT_ERROR_STATE_INVALID       (TRANSPORT_ERROR_STATE | TRANSPORT_MODULE_CORE | 0x01U)
	#define TRANSPORT_ERROR_STATE_CONFLICT      (TRANSPORT_ERROR_STATE | TRANSPORT_MODULE_CORE | 0x02U)
	#define TRANSPORT_ERROR_STATE_UNINITIALIZED (TRANSPORT_ERROR_STATE | TRANSPORT_MODULE_CORE | 0x03U)

	// Error info structure
	typedef struct {
		TransportResult error_code;
		char error_message[256];
		char function_name[64];
		uint32_t timestamp;
		uint32_t line_number;
	} TransportErrorInfo;

	/// Create a Transport object
	TRANSPORT_API TransportResult transport_create(const struct hid_device_info* device_info, TransportHandle* out_handle);
	/// Destroy Transport object
	TRANSPORT_API TransportResult transport_destroy(TransportHandle handle);

	/// Firmware version
	TRANSPORT_API TransportResult transport_get_firmware_version(TransportHandle handle, char* firmware_version_string, size_t length);

	/// Clear task queue
	TRANSPORT_API TransportResult transport_clear_task_queue(TransportHandle handle);

	/// Control functions
	TRANSPORT_API TransportResult transport_wakeup_screen(TransportHandle handle);
	TRANSPORT_API TransportResult transport_set_keyboard_backlight_brightness(TransportHandle handle, uint8_t brightness);
	TRANSPORT_API TransportResult transport_set_keyboard_lighting_effects(TransportHandle handle, uint8_t effect);
	TRANSPORT_API TransportResult transport_set_keyboard_lighting_speed(TransportHandle handle, uint8_t speed);
	TRANSPORT_API TransportResult transport_set_keyboard_rgb_backlight(TransportHandle handle, uint8_t red, uint8_t green, uint8_t blue);
	TRANSPORT_API TransportResult transport_keyboard_os_mode_switch(TransportHandle handle, uint8_t os_mode_enum);
	TRANSPORT_API TransportResult transport_set_key_brightness(TransportHandle handle, uint8_t brightness);
	TRANSPORT_API TransportResult transport_clear_all_keys(TransportHandle handle);
	TRANSPORT_API TransportResult transport_clear_key(TransportHandle handle, uint8_t key_value);
	TRANSPORT_API TransportResult transport_refresh(TransportHandle handle);
	TRANSPORT_API TransportResult transport_sleep(TransportHandle handle);
	TRANSPORT_API TransportResult transport_disconnected(TransportHandle handle);
	TRANSPORT_API TransportResult transport_heartbeat(TransportHandle handle);

	//TRANSPORT_API void transport_set_key_bitmap(TransportHandle handle, const char *bitmap, size_t length, uint8_t key_value);
	TRANSPORT_API TransportResult transport_set_background_bitmap(TransportHandle handle, const char* bitmap, size_t length, uint32_t timeout_ms);

	//TRANSPORT_API void transport_set_key_image(TransportHandle handle, const char *file_path, uint8_t key_value);
	//TRANSPORT_API void transport_set_background_image(TransportHandle handle, const char *file_path, uint32_t timeout_ms);

	TRANSPORT_API TransportResult transport_set_key_image_stream(TransportHandle handle, const char* jpeg, size_t length, uint8_t key_value);
	TRANSPORT_API TransportResult transport_set_background_image_stream(TransportHandle handle, const char* jpeg, size_t length, uint32_t timeout_ms);

	TRANSPORT_API TransportResult transport_set_background_frame_stream(TransportHandle handle, const char* jpeg, size_t length, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t FBlayer);
	TRANSPORT_API TransportResult transport_clear_background_frame_stream(TransportHandle handle, uint8_t postion);

	/// LED control
	TRANSPORT_API TransportResult transport_set_led_brightness(TransportHandle handle, uint8_t brightness);
	TRANSPORT_API TransportResult transport_set_led_color(TransportHandle handle, uint16_t count, uint8_t r, uint8_t g, uint8_t b);
	TRANSPORT_API TransportResult transport_reset_led_color(TransportHandle handle);

	/// Device configuration
	TRANSPORT_API TransportResult transport_set_device_config(TransportHandle handle, uint8_t* configs, size_t configs_length);

	/// N1 mode switch
	TRANSPORT_API TransportResult transport_change_mode(TransportHandle handle, uint8_t mode);

	/// Status
	TRANSPORT_API TransportResult transport_can_write(TransportHandle handle, int* can_write);
	TRANSPORT_API TransportResult transport_read(TransportHandle handle, uint8_t* response, size_t* length, int32_t timeoutMs);
	TRANSPORT_API TransportResult transport_set_reportID(TransportHandle handle, uint8_t reportID);
	TRANSPORT_API TransportResult transport_reportID(TransportHandle handle, uint8_t* out_reportID);
	TRANSPORT_API TransportResult transport_set_reportSize(TransportHandle handle, uint16_t input_report_size, uint16_t output_report_size, uint16_t feature_report_size);
	TRANSPORT_API TransportResult transport_raw_hid_last_error(TransportHandle handle, wchar_t* errMsg, size_t* length);
	TRANSPORT_API TransportResult transport_disable_output(int8_t disable_output);

	/// Get the last error info
	TRANSPORT_API TransportResult transport_get_last_error_info(TransportHandle handle, TransportErrorInfo* error_info);
	/// Export hidapi functions to avoid conflicts with other packages, such as Python's hidapi package
	TRANSPORT_API struct hid_device_info* transport_hid_enumerate(unsigned short vendor_id, unsigned short product_id);
	TRANSPORT_API void transport_hid_free_enumeration(struct hid_device_info* devs);

#ifdef __cplusplus
}
#endif

#endif // _TRANSPORT_C_H_
