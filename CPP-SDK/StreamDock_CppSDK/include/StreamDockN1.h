#pragma once
#include "StreamDock.h"

class StreamDockN1 : public StreamDock
{
public:
	StreamDockN1(const std::shared_ptr<TranSport>& transport, struct hid_device_info* copyDevInfo);
	~StreamDockN1();
	virtual void init(int brightness = 100) override;
	virtual int setBackgroundImg(const std::string& path) override;
	virtual int setBackgroundImgData(unsigned char* imagedata, RgbaFormat pitch = RgbaFormat::RGB, unsigned int loadPicWidth = 0, unsigned int loadPicHeight = 0) override;
	virtual int setKeyImg(const std::string& path, int key) override;
	virtual int setKeyImgData(unsigned char* imagedata, int key, RgbaFormat pitch = RgbaFormat::RGB, unsigned int loadPicWidth = 0, unsigned int loadPicHeight = 0) override;
public:
	virtual int switchMode(N1MODE mode) override;
private:
	N1MODE mode = N1MODE::SOFTWARE_MODE;
};
