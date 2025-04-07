#pragma once
#include <iostream>
#include "hidapi.h"
#include <sys/stat.h>
#include <fstream>
#include <vector>
#include <future>

enum class N1MODE {
	KEYBOARD_MODE,
	CALCULATOR_MODE,
	SOFTWARE_MODE
};

enum class ErrorCode
{
	UnKnown,
	MALLOC_ERR,
	NULLPTR_ERR,
	FILEOPEN_ERR,
	JPG_FORMAT_ERR,
	IMGSIZE_EXCEEDS_ERR,
	PENDING,
	HIDOPEN_ERR,
	HIDWRITE_ERR,
	HIDINRERNAL_ERR
};
enum class HidStatus
{
	Inited,
	Exited
};

struct hid_device_info_store
{
	hid_device_info_store(hid_device_info *info)
	{
		if (info->path)
			path = info->path;
		vendor_id = info->vendor_id;
		product_id = info->product_id;
		if (info->serial_number)
			serial_number = info->serial_number;
		if (info->product_string)
			product_string = info->product_string;
		usage = info->usage;
		usage_page = info->usage_page;
		interface_number = info->interface_number;
		bus_type = info->bus_type;
		release_number = info->release_number;
	};
	std::string path;
	unsigned short vendor_id;
	unsigned short product_id;
	std::wstring serial_number;
	unsigned short release_number;
	std::wstring manufacturer_string;
	std::wstring product_string;
	unsigned short usage_page;
	unsigned short usage;
	int interface_number;
	hid_bus_type bus_type;
};

struct hid_device_info_RAII
{
	explicit hid_device_info_RAII(hid_device_info *info) : _info(info) {}
	hid_device_info_RAII(const hid_device_info_RAII &) = delete;
	hid_device_info_RAII &operator=(const hid_device_info_RAII &) = delete;
	hid_device_info_RAII(hid_device_info_RAII &&other) noexcept : _info(other._info)
	{
		other._info = nullptr;
	}

	hid_device_info_RAII &operator=(hid_device_info_RAII &&other) noexcept
	{
		if (this != &other)
		{
			if (_info)
			{
				hid_free_enumeration(_info);
			}
			_info = other._info;
			other._info = nullptr;
		}
		return *this;
	}
	~hid_device_info_RAII()
	{
		if (_info)
		{
			hid_free_enumeration(_info);
		}
	}
	hid_device_info *_info = nullptr;
};
class TranSport
{
public:
	explicit TranSport();
	~TranSport();
	int open(const char *path, bool auto_read = true);
	int getInputReport(unsigned char *report, const int length);
	int read(unsigned char *data, unsigned long length);
	void startAutoRead();
	void stopAutoRead();
	int write(unsigned char *data, unsigned long length);
	int freeEnumerate(hid_device_info *devs);
	hid_device_info *enumerate(const unsigned short vid, const unsigned short pid);
	hid_device_info_RAII enumerateEx(const unsigned short vid, const unsigned short pid);
	int setBrightness(unsigned int percent);
	int setBackgroundImg(unsigned char *buffer, const int size);
	int setKeyImg(const std::string &path, const int key);
	int setKeyImgData(unsigned char *data, const int key, const int size);
	int setBackgroundImgDualDevice(const std::string &path);
	int setKeyImgDualDevice(const std::string &path, const int key);
	int setKeyImgDataDualDevice(const std::string &, const int key);
	int keyClear(const int key);
	int keyAllClear();
	int wakeScreen();
	int refresh();
	int disconnected();
	void close();
	ErrorCode code() const;
	HidStatus status() const;
	int switchMode(N1MODE mode);
private:
	hid_device *handle = nullptr;
	HidStatus m_status = HidStatus::Exited;
	ErrorCode m_code = ErrorCode::UnKnown;
	// var for auto read
	bool m_autoReadFlag = false;		 // if start auto read function ?
	std::future<void> m_future;			 // save the async task
	std::atomic<bool> m_stop{true};		 // if auto read loop stoped ?
	std::atomic<bool> m_ready{true};	 // if device is ready to get command ?
	std::mutex m_mtx;					 // block the program until device is ready
	std::condition_variable m_condition; // block the program until device is ready
	// after set logo, block some time
	unsigned int blockTIme_new = 50;   // we expect a time 10ms - 100ms in new platform
	unsigned int blockTIme_old = 3000; // we expect a time 10ms - 100ms in old platform
};
