#include "StreamDock293V3.h"

StreamDock293V3::StreamDock293V3(const std::shared_ptr<TranSport> &transport, struct hid_device_info *copyDevInfo) : StreamDock(transport, copyDevInfo)
{
    setKeyFormat(112, 112);
    setBgiFormat(800, 480);
    setRotate(180.0f, 180.0f);
    setKeyRange(1, 15);
}

StreamDock293V3::~StreamDock293V3()
{
}

int StreamDock293V3::transform(int x)
{
    if (x >= 1 && x <= 5)
    {
        return x + 10;
    }
    else if (x >= 6 && x <= 10)
    {
        return x;
    }
    else if (x >= 11 && x <= 15)
    {
        return x - 10;
    }
    else
        return -1;
}

int StreamDock293V3::setBackgroundImg(const std::string &path)
{
    try
    {
        fipImage img;
        if (img.load(path.c_str()))
        {
            const std::string tmpFileName = "setBackgroundImg_tmp.jpg";
            PicHelper::toNativeBackground(img, this);
            int size = bg_width * bg_height * 3;
            img.save(FIF_JPEG, tmpFileName.c_str(), 90);
            auto ret = this->transport->setBackgroundImgDualDevice(tmpFileName.c_str());
            std::remove(tmpFileName.c_str());
            return ret;
        }
        else
            throw std::runtime_error("Failed to load image from: " + path);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in setBackgroundImg: " << e.what() << std::endl;
        return 0;
    }
}

int StreamDock293V3::setBackgroundImgData(unsigned char *imagedata, RgbaFormat pitch, unsigned int loadPicWidth, unsigned int loadPicHeight)
{
    try
    {
        if (!imagedata)
            throw std::invalid_argument("imagedata is null.");
        if (loadPicWidth == 0 || loadPicHeight == 0)
            throw std::invalid_argument("Invalid image dimensions: width or height is zero.");
        if (static_cast<int>(pitch) <= 0)
            throw std::invalid_argument("Invalid pitch value.");

        FIBITMAP *bitmap = FreeImage_ConvertFromRawBits(imagedata, loadPicWidth, loadPicHeight, loadPicWidth * static_cast<int>(pitch), static_cast<int>(pitch) * 8, 0xFF0000, 0x00FF00, 0x0000FF, false);
        fipImage img;
        img = bitmap; // manage bitmap, can't deconstruct
        PicHelper::toNativeBackground(img, this);
        const std::string tmpFileName = "setBackgroundImgData_tmp.jpg";
        img.save(FIF_JPEG, tmpFileName.c_str(), 90);
        auto ret = this->transport->setBackgroundImgDualDevice(tmpFileName);
        std::remove(tmpFileName.c_str());
        return ret;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in setBackgroundImgData: " << e.what() << std::endl;
        return 0;
    }
}

int StreamDock293V3::setKeyImg(const std::string &path, int key)
{
    try
    {
        fipImage img;
        if (img.load(path.c_str()))
        {
            const std::string tmpFileName = "setKeyImg_tmp.jpg";
            PicHelper::toNativeKeyImage(img, this);
            int size = ky_width * ky_height * 3;
            img.save(FIF_JPEG, tmpFileName.c_str(), 90);
            auto ret = this->transport->setKeyImgDualDevice(tmpFileName.c_str(), transform(key));
            std::remove(tmpFileName.c_str());
            return ret;
        }
        else
            throw std::runtime_error("Failed to load image from: " + path);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in setKeyImg: " << e.what() << std::endl;
        return 0;
    }
}

int StreamDock293V3::setKeyImgData(unsigned char *imagedata, int key, RgbaFormat pitch, unsigned int loadPicWidth, unsigned int loadPicHeight)
{
    try
    {
        if (!imagedata)
            throw std::invalid_argument("imagedata is null.");
        if (loadPicWidth == 0 || loadPicHeight == 0)
            throw std::invalid_argument("Invalid image dimensions: width or height is zero.");
        if (static_cast<int>(pitch) <= 0)
            throw std::invalid_argument("Invalid pitch value.");

        FIBITMAP *bitmap = FreeImage_ConvertFromRawBits(imagedata, loadPicWidth, loadPicHeight, loadPicWidth * static_cast<int>(pitch), static_cast<int>(pitch) * 8, 0xFF0000, 0x00FF00, 0x0000FF, false);
        fipImage img;
        img = bitmap; // manage bitmap, can't deconstruct
        PicHelper::toNativeKeyImage(img, this);
        const std::string tmpFileName = "setKeyImgData_tmp.jpg";
        img.save(FIF_JPEG, tmpFileName.c_str(), 90);
        auto ret = this->transport->setKeyImgDataDualDevice(tmpFileName, transform(key));
        std::remove(tmpFileName.c_str());
        return ret;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in setKeyImgData: " << e.what() << std::endl;
        return 0;
    }
}

int StreamDock293V3::clearIcon(int index)
{
    int key = transform(index);
    return this->transport->keyClear(key);
}