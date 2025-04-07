# -*- coding: utf-8 -*-
from .StreamDock import StreamDock
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random

class StreamDockN1(StreamDock):
    KEY_MAP = False
    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)

    def open(self):
        super().open()
        self.transport.switchMode(2)
        
    # 设置设备的屏幕亮度
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)
    
    def set_touchscreen_image(self, path):
        pass

    # 设置设备的按键图标 96 * 96
    def set_key_image(self, key, path):
        try:
            # assert
            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1
            if key not in range(1, 19):
                print(f"key '{key}' out of range. you should set (1 ~ 18)")
                return -1
            image = Image.open(path)
            if key in range(1 , 16):
                # icon
                rotated_image = to_native_key_format(self, image)
            elif key in range(16, 19):
                # second screen
                rotated_image = to_native_seondscreen_format(self, image)
            rotated_image.save("Temporary.jpg", "JPEG", subsampling=0, quality=90)
            returnvalue = self.transport.setKeyImgDualDevice(bytes("Temporary.jpg",'utf-8'), key)
            os.remove("Temporary.jpg")
            return returnvalue

        except Exception as e:
            print(f"Error: {e}")
            return -1

    def get_serial_number(self, lenth):
        return self.transport.getInputReport(lenth)
    
    def switch_mode(self, mode):
        return self.transport.switchMode(mode)

    def key_image_format(self):
        return {
            'size': (96, 96),
            'format': "JPEG",
            'rotation': 0,
            'flip': (False, False)
        }

    def secondscreen_image_format(self):
        return {
            'size': (64, 64),
            'format': "JPEG",
            'rotation': 0,
            'flip': (False, False)
        }
        
