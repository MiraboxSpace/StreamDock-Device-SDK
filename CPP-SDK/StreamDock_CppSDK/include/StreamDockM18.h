#pragma once
#include "StreamDock.h"

class StreamDockM18 : public StreamDock
{
public:
    StreamDockM18(const std::shared_ptr<TranSport> &transport, struct hid_device_info *copyDevInfo);
    ~StreamDockM18();
    int transform(int x);
    int setBackgroundImg(const std::string &path);
    int setBackgroundImgData(unsigned char *imagedata, RgbaFormat pitch = RgbaFormat::RGB, unsigned int loadPicWidth = 0, unsigned int loadPicHeight = 0);
    int setKeyImg(const std::string &path, int key);
    int setKeyImgData(unsigned char *imagedata, int key, RgbaFormat pitch = RgbaFormat::RGB, unsigned int loadPicWidth = 0, unsigned int loadPicHeight = 0);
    int clearIcon(int index);
};
