#pragma once
#include "StreamDock.h"

class StreamDock293 : public StreamDock
{
private:
public:
	StreamDock293(const std::shared_ptr<TranSport> &transport, struct hid_device_info *copyDevInfo);
	~StreamDock293();

	int transform(int x);
	virtual int setBackgroundImg(const std::string &path) override;
	virtual int setBackgroundImgData(unsigned char *imagedata, RgbaFormat pitch = RgbaFormat::RGB, unsigned int loadPicWidth = 0, unsigned int loadPicHeight = 0) override;
	virtual int setKeyImg(const std::string &path, int key) override;
	virtual int setKeyImgData(unsigned char *imagedata, int key, RgbaFormat pitch = RgbaFormat::RGB, unsigned int loadPicWidth = 0, unsigned int loadPicHeight = 0) override;
	virtual int clearIcon(int index) override;
};
