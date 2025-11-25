#pragma once
#include "DeviceInfo/streamdock.h"
#include "DeviceInfo/streamdockfactory.h"

int extract_last_number(const std::string &code)
{
	// 找到最后一个点的位置
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

class StreamDockN1 : public StreamDock
{
public:
	enum class N1MODE
	{
		KEYBOARD_MODE,
		CALCULATOR_MODE,
		SOFTWARE_MODE
	};

public:
	explicit StreamDockN1(const hid_device_info &device_info);
	virtual RegisterEvent dispatchEvent(uint8_t readValue, uint8_t eventValue) override;
	virtual void setBackgroundImgFile(const std::string &filePath, uint32_t timeoutMs = 3000) override
	{
		if (extract_last_number(info()->firmwareVersion) >= 13)
			StreamDock::setBackgroundImgFile(filePath, timeoutMs);
	}
	virtual void setBackgroundImgStream(const std::string &stream, uint32_t timeoutMs = 3000) override
	{
		if (extract_last_number(info()->firmwareVersion) >= 13)
			StreamDock::setBackgroundImgStream(stream, timeoutMs);
	}
	void changeMode(N1MODE mode);

private:
	static bool registered_N1;
	static bool registered_N1E;
};