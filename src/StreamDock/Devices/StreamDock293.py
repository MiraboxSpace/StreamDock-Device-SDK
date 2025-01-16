from .StreamDock import StreamDock
from PIL import Image
import ctypes
import ctypes.util
import os, io

class StreamDock293(StreamDock):        
    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)

    # 设置设备的屏幕亮度
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)
    

    # 设置设备的背景图片 800 * 480
    def set_touchscreen_image(self, image):

        image = Image.open(image)
        # 转换为RGB模式
        image = image.convert('RGB')
        width,height=image.size
        # 获取图片的像素数据
        pixels = list(image.getdata())

        bgr_data = []
    
        # 遍历像素数据，将RGB值转换为BGR值并添加到列表中
        for pixel in pixels:
            r, g, b = pixel
            bgr_data.extend([r, g, b])
        arr_type = ctypes.c_char * len(bgr_data)
        arr_ctypes = arr_type(*bgr_data)
        reversed_bgr_array = arr_ctypes[::-1]
        return self.transport.setBackgroundImg(reversed_bgr_array,width*height*3)
    
    # 设置设备的按键图标 100 * 100
    def set_key_image(self, key, image_buff):
        if isinstance(image_buff, str):
            self.transport.setKeyImg(bytes(image_buff,'utf-8'), key)
            return
        return self.set_key_imageData(key, image_buff)

    #向图片传入二进制数据，width：图片的宽默认100，height：图片的高默认100
    # def set_key_imageData(self, key, image_buff, width=100, height=100):
    #     return self.transport.setKeyImgdata(image_buff, key, width, height)
    
    # 获取设备的固件版本号
    def get_serial_number(self,length):
        return self.transport.getInputReport(length)

    def key_image_format(self):
        return {
            'size': (100, 100),
            'format': "JPEG",
            'rotation': 180,
            'flip': (False, False)
        }