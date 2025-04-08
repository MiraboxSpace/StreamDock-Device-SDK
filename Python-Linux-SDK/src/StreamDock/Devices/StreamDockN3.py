from .StreamDock import StreamDock
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random

class StreamDockN3(StreamDock):        
    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)
        self.KEY_MAP = False

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

            # open formatter
            image = Image.open(path)
            image = to_native_touchscreen_format(self, image)
            temp_image_path = "rotated_touchscreen_image_" + str(random.randint(9999, 999999)) + ".jpg"
            image.save(temp_image_path)
            
            # encode send
            path_bytes = temp_image_path.encode('utf-8')  
            c_path = ctypes.c_char_p(path_bytes) 
            res = self.transport.setBackgroundImgDualDevice(c_path)
            os.remove(temp_image_path)
            return res
        
        except Exception as e:
            print(f"Error: {e}")
            return -1
        
    # 设置设备的按键图标 112 * 112
    def set_key_image(self, key, path):
        try:
            # assert
            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1
            
            # open formatter
            image = Image.open(path)
            image = to_native_key_format(self, image)
            temp_image_path = "rotated_key_image_" + str(random.randint(9999, 999999)) + ".jpg"
            image.save(temp_image_path)

            # encode send
            path_bytes = temp_image_path.encode('utf-8') 
            c_path = ctypes.c_char_p(path_bytes)  
            res = self.transport.setKeyImgDualDevice(c_path, key)
            os.remove(temp_image_path)
            return res
            
        except Exception as e:
            print(f"Error: {e}")
            return -1

    # 待补充
    def set_key_imageData(self, key, path):
        pass
    
    # 获取设备的固件版本号
    def get_serial_number(self,length):
        return self.transport.getInputReport(length)

    def key_image_format(self):
        return {
            'size': (64, 64),
            'format': "JPEG",
            'rotation': -90,
            'flip': (False, False)
        }
        
    def touchscreen_image_format(self):
        return {
            'size': (320, 240),
            'format': "JPEG",
            'rotation': -90,
            'flip': (False, False)
        }