from .StreamDock import StreamDock
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random

KEY_MAPPING = {
    1: 25,
    2: 26,
    3: 27,
    4: 28,
    5: 29,
    6: 30,
    7: 31,
    8: 32,
    9: 17,
    10: 18,
    11: 19,
    12: 20,
    13: 21,
    14: 22,
    15: 23,
    16: 24,
    17: 9,
    18: 10,
    19: 11,
    20: 12,
    21: 13,
    22: 14,
    23: 15,
    24: 16,
    25: 1,
    26: 2,
    27: 3,
    28: 4,
    29: 5,
    30: 6,
    31: 7,
    32: 8,
}


class StreamDockXL(StreamDock):
    KEY_MAP = False

    def key(self, k):
        if k in range(1, 33):
            return KEY_MAPPING[k]
        else:
            return k

    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)

    # 设置设备的屏幕亮度
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)

    # 设置设备的背景图片 1024 * 600
    def set_touchscreen_image(self, path):
        try:
            # assert
            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1

            # open formatter
            image = Image.open(path)
            image = to_native_touchscreen_format(self, image)
            temp_image_path = (
                "rotated_touchscreen_image_"
                + str(random.randint(9999, 999999))
                + ".jpg"
            )
            image.save(temp_image_path)

            # encode send
            path_bytes = temp_image_path.encode("utf-8")
            c_path = ctypes.c_char_p(path_bytes)
            res = self.transport.setBackgroundImgDualDevice(c_path)
            os.remove(temp_image_path)
            return res

        except Exception as e:
            print(f"Error: {e}")
            return -1

    # 设置设备的按键图标 80 * 80
    def set_key_image(self, key, path):
        try:
            origin = key
            key = self.key(key)
            if origin not in range(1, 33):
                print(f"key '{origin}' out of range. you should set (1 ~ 32)")
                return -1
            # open formatter
            image = Image.open(path)
            image = to_native_key_format(self, image)
            temp_image_path = (
                "rotated_key_image_" + str(random.randint(9999, 999999)) + ".jpg"
            )
            image.save(temp_image_path)

            # encode send
            path_bytes = temp_image_path.encode("utf-8")
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
    def get_serial_number(self):
        return self.serial_number

    def key_image_format(self):
        return {
            "size": (80, 80),
            "format": "JPEG",
            "rotation": 180,
            "flip": (False, False),
        }

    def touchscreen_image_format(self):
        return {
            "size": (1024, 600),
            "format": "JPEG",
            "rotation": 180,
            "flip": (False, False),
        }

    # 设置设备参数
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.hasRGBLed = True
        self.feature_option.ledCounts = 6
        pass
