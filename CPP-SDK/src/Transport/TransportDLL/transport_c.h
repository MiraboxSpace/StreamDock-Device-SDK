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

	/// 创建 Transport 对象
	TRANSPORT_API TransportHandle transport_create(const struct hid_device_info* device_info);
	/// 销毁 Transport 对象
	TRANSPORT_API void transport_destroy(TransportHandle handle);

	/// 固件号
	TRANSPORT_API void transport_get_firmware_version(TransportHandle handle, char* firmware_version_string, size_t length);

	/// 清空任务队列
	TRANSPORT_API void transport_clear_task_queue(TransportHandle handle);

	/// 控制函数
	TRANSPORT_API void transport_wakeup_screen(TransportHandle handle);
	TRANSPORT_API void transport_set_key_brightness(TransportHandle handle, uint8_t brightness);
	TRANSPORT_API void transport_clear_all_keys(TransportHandle handle);
	TRANSPORT_API void transport_clear_key(TransportHandle handle, uint8_t key_value);
	TRANSPORT_API void transport_refresh(TransportHandle handle);
	TRANSPORT_API void transport_sleep(TransportHandle handle);
	TRANSPORT_API void transport_disconnected(TransportHandle handle);
	TRANSPORT_API void transport_heartbeat(TransportHandle handle);

	//TRANSPORT_API void transport_set_key_bitmap(TransportHandle handle, const char *bitmap, size_t length, uint8_t key_value);
	TRANSPORT_API void transport_set_background_bitmap(TransportHandle handle, const char* bitmap, size_t length, uint32_t timeout_ms);

	//TRANSPORT_API void transport_set_key_image(TransportHandle handle, const char *file_path, uint8_t key_value);
	//TRANSPORT_API void transport_set_background_image(TransportHandle handle, const char *file_path, uint32_t timeout_ms);

	TRANSPORT_API void transport_set_key_image_stream(TransportHandle handle, const char* jpeg, size_t length, uint8_t key_value);
	TRANSPORT_API void transport_set_background_image_stream(TransportHandle handle, const char* jpeg, size_t length, uint32_t timeout_ms);

	TRANSPORT_API void transport_set_background_frame_stream(TransportHandle handle, const char* jpeg, size_t length, uint16_t width, uint16_t height, uint16_t x, uint16_t y, uint8_t FBlayer);
	TRANSPORT_API void transport_clear_background_frame_stream(TransportHandle handle, uint8_t postion);

	/// LED 控制
	TRANSPORT_API void transport_set_led_brightness(TransportHandle handle, uint8_t brightness);
	TRANSPORT_API void transport_set_led_color(TransportHandle handle, uint16_t count, uint8_t r, uint8_t g, uint8_t b);
	TRANSPORT_API void transport_reset_led_color(TransportHandle handle);

	/// 设备配置
	TRANSPORT_API void transport_set_device_config(TransportHandle handle, uint8_t* configs, size_t configs_length);

	/// N1 模式转化
	TRANSPORT_API void transport_change_mode(TransportHandle handle, uint8_t mode);

	/// 状态
	TRANSPORT_API int transport_can_write(TransportHandle handle);
	TRANSPORT_API int transport_read(TransportHandle handle, uint8_t* response, size_t* length, int32_t timeoutMs);
	TRANSPORT_API void transport_set_reportID(TransportHandle handle, uint8_t reportID);
	TRANSPORT_API uint8_t transport_reportID(TransportHandle handle);
	TRANSPORT_API void transport_set_reportSize(TransportHandle handle, uint16_t input_report_size, uint16_t output_report_size, uint16_t feature_report_size);
	TRANSPORT_API void transport_raw_hid_last_error(TransportHandle handle, wchar_t* errMsg, size_t* length);
	TRANSPORT_API void transport_disable_output(int8_t disable_output);

#ifdef __cplusplus
}
#endif

#endif // _TRANSPORT_C_H_
