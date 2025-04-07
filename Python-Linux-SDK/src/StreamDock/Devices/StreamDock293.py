from .StreamDock import StreamDock
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random

class StreamDock293(StreamDock):    
    KEY_MAP = True
    def __init__(self, transport1, devInfo): 
        super().__init__(transport1, devInfo)

    # 设置设备的屏幕亮度
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)
    

    # 设置设备的背景图片 800 * 480
    def set_touchscreen_image(self, path):
        try:
            # assert
            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1
            image = Image.open(path)
            image = to_native_touchscreen_format(self, image)
            width, height = image.size
            bgr_data = []
            for y in range(height):
                for x in range(width):
                
                    r,g,b = image.getpixel((x,y))
                    bgr_data.extend([b,g,r])
            arr_type = ctypes.c_char * len(bgr_data)
            arr_ctypes = arr_type(*bgr_data)
            return self.transport.setBackgroundImg(ctypes.cast(arr_ctypes, ctypes.POINTER(ctypes.c_ubyte)),width * height * 3)
        
        except Exception as e:
            print(f"Error: {e}")
            return -1
    
    # 设置设备的按键图标 100 * 100
    def set_key_image(self, key, path):
        try:
            origin = key
            key = self.key(key)
            # assert
            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1
            if origin not in range(1, 16):
                print(f"key '{origin}' out of range. you should set (1 ~ 15)")
                return -1
            
            image = Image.open(path)
            rotated_image = to_native_key_format(self, image)

            rotated_image.save("Temporary.jpg", "JPEG", subsampling=0, quality=100)
            returnvalue = self.transport.setKeyImg(bytes("Temporary.jpg",'utf-8'), key)
            os.remove("Temporary.jpg")
            return returnvalue

        except Exception as e:
            print(f"Error: {e}")
            return -1
    
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
    
    def touchscreen_image_format(self):
        return {
            'size': (800, 480),
            'format': "JPEG",
            'rotation': 180,
            'flip': (False, False)
        }
    