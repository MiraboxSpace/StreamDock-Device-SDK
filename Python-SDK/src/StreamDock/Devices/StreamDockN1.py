# -*- coding: utf-8 -*-
from .StreamDock import StreamDock
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random

def extract_last_number(code):
    """
    从字符串中提取最后一个点后面的连续数字并转为整数
    
    Args:
        code: 格式如 "N3.02.013" 或 "N3.02.013V2" 的字符串
    
    Returns:
        int: 提取的数字，如果找不到则返回 None
    """
    # 找到最后一个点的位置
    last_dot = code.rfind('.')
    if last_dot == -1:
        return None
    
    # 从最后一个点后面开始提取连续数字
    num_str = ''
    for char in code[last_dot + 1:]:
        if char.isdigit():
            num_str += char
        else:
            # 遇到第一个非数字字符就停止
            break
    
    # 如果提取到了数字，转为整数
    if num_str:
        return int(num_str)
    else:
        return None

class StreamDockN1(StreamDock):
    KEY_MAP = False
    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)
        self.devInfo=devInfo

    def open(self):
        super().open()
        self.transport.switchMode(2)
        
    # 设置设备的屏幕亮度
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)
    
    # 设置设备的背景图片 480 * 854
    def set_touchscreen_image(self, path):
        version_str=self.get_serial_number();
        version_num=extract_last_number(version_str)
        if version_num is None:
            return -1
        if(version_num>=13):
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

    def get_serial_number(self):
        return self.serial_number
    
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
    def touchscreen_image_format(self):
        return {
            'size': (480, 854),
            'format': "JPEG",
            'rotation': 0,
            'flip': (False, False)
        }
    # 设置设备参数
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        pass