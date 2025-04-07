#include "StreamDock293.h"
#include <filesystem>
namespace fs = std::filesystem;

StreamDock293::StreamDock293(const std::shared_ptr<TranSport> &transport, struct hid_device_info *copyDevInfo) : StreamDock(transport, copyDevInfo)
{
    setKeyFormat(100, 100);
    setBgiFormat(800, 480);
    setRotate(180.0f, 0.0f);
    setFlipHorizonal(true, false);
    setKeyRange(1, 15);
}

StreamDock293::~StreamDock293()
{
}

int StreamDock293::transform(int x)
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
    return -1;
}

int StreamDock293::setBackgroundImg(const std::string &path)
{
    try
    {
        fipImage img;
        if (img.load(path.c_str()))
        {
            PicHelper::toNativeBackground(img, this);
            int size = bg_width * bg_height * 3;
            return this->transport->setBackgroundImg(img.accessPixels(), size);
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

int StreamDock293::setBackgroundImgData(unsigned char *imagedata, RgbaFormat pitch, unsigned int loadPicWidth, unsigned int loadPicHeight)
{
    try
    {
        if (!imagedata)
            throw std::invalid_argument("imagedata is null.");
        if (loadPicWidth == 0 || loadPicHeight == 0)
            throw std::invalid_argument("Invalid image dimensions: width or height is zero.");
        if (static_cast<int>(pitch) <= 0)
            throw std::invalid_argument("Invalid pitch value.");

        FIBITMAP *bitmap = FreeImage_ConvertFromRawBits(imagedata, loadPicWidth, loadPicHeight, loadPicWidth * static_cast<int>(pitch),
                                                        static_cast<int>(pitch) * 8, 0xFF0000, 0x00FF00, 0x0000FF, false);
        if (!bitmap)
            throw std::runtime_error("Failed to convert raw image data to FIBITMAP.");
        fipImage img;
        img = bitmap;
        PicHelper::toNativeBackground(img, this);
        int size = loadPicWidth * loadPicHeight * 3;
        return this->transport->setBackgroundImg(img.accessPixels(), size);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in setBackgroundImgData: " << e.what() << std::endl;
        return 0;
    }
}

int StreamDock293::setKeyImg(const std::string &path, int key)
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
            auto ret = this->transport->setKeyImg(tmpFileName, transform(key));
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

int StreamDock293::setKeyImgData(unsigned char *imagedata, int key, RgbaFormat pitch, unsigned int loadPicWidth, unsigned int loadPicHeight)
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
        fipMemoryIO memIO; // get jpg data in memory
        img.saveToMemory(FIF_JPEG, memIO, 90);
        unsigned char *memBuffer = nullptr;
        DWORD size = 0;
        FreeImage_AcquireMemory(memIO, &memBuffer, &size);
        return this->transport->setKeyImgData(memBuffer, transform(key), size);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in setKeyImgData: " << e.what() << std::endl;
        return 0;
    }
}

int StreamDock293::clearIcon(int index)
{
    int key = transform(index);
    return this->transport->keyClear(key);
}
