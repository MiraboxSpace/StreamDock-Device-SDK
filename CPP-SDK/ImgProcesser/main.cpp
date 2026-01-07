#include "Gif2ImgFrame.h"
#include "OpenCVImageEncoder.h"
#include "OpenCVSupportFormat.h"
#include <iostream>

int main() {
    Gif2ImgFrame gif("234.gif", new OpenCVImageEncoder);

    if (!gif.isValid()) {
        std::cerr << "Failed to load" << std::endl;
        return 1;
    }

    gif.saveFramesToFiles("out_png", ImgType::JPG);
    auto data = gif.encodeFramesToMemory(ImgType::JPG, 90);

    for (size_t i = 0; i < data.size(); ++i){
         std::cout << "Frame " << i << ": " << data[i].size() << " bytes" << std::endl;
        //IImageEncoder::saveImageToFile(data[i], "./out_png/" + std::to_string(i) + ".jpg");
    }

    return 0;
}
