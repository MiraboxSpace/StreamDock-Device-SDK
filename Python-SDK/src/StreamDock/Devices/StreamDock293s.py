# -*- coding: utf-8 -*-
from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import InputEvent, ButtonKey, EventType
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random


class StreamDock293s(StreamDock):
    """StreamDock293s 设备类 - 支持15个按键和3个副屏按键"""

    KEY_COUNT = 18
    KEY_MAP = False

    # 图片键映射：逻辑键 -> 硬件键（用于设置图片）
    _IMAGE_KEY_MAP = {
        ButtonKey.KEY_1: 13,
        ButtonKey.KEY_2: 10,
        ButtonKey.KEY_3: 7,
        ButtonKey.KEY_4: 4,
        ButtonKey.KEY_5: 1,
        ButtonKey.KEY_6: 14,
        ButtonKey.KEY_7: 11,
        ButtonKey.KEY_8: 8,
        ButtonKey.KEY_9: 5,
        ButtonKey.KEY_10: 2,
        ButtonKey.KEY_11: 15,
        ButtonKey.KEY_12: 12,
        ButtonKey.KEY_13: 9,
        ButtonKey.KEY_14: 6,
        ButtonKey.KEY_15: 3,
        # 副屏按键 16-18
        ButtonKey.KEY_16: 16,
        ButtonKey.KEY_17: 17,
        ButtonKey.KEY_18: 18,
    }

    # 反向映射：硬件键 -> 逻辑键（用于事件解码）
    _HW_TO_LOGICAL_KEY = {v: k for k, v in _IMAGE_KEY_MAP.items()}

    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)

    def get_image_key(self, logical_key: ButtonKey) -> int:
        """
        将逻辑键值转换为硬件键值（用于设置图片）

        Args:
            logical_key: 逻辑键值枚举

        Returns:
            int: 硬件键值
        """
        if logical_key in self._IMAGE_KEY_MAP:
            return self._IMAGE_KEY_MAP[logical_key]
        raise ValueError(f"StreamDock293s: 不支持的按键 {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        将硬件事件码解码为统一的 InputEvent

        293s 设备只支持普通按键，硬件码范围 1-18
        """
        # 处理状态值：0x02=释放, 0x01=按下
        normalized_state = 1 if state == 0x01 else 0

        # 普通按键事件 (1-18)
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state
            )

        # 未知事件
        return InputEvent(event_type=EventType.UNKNOWN)

    # 设置设备的屏幕亮度
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)

    # 设置设备的背景图片  854 * 480
    def set_touchscreen_image(self, image):
        image = Image.open(image)
        image = to_native_touchscreen_format(self, image)
        width, height = image.size
        bgr_data = []

        for x in range(width):
            for y in range(height):
                r,g,b = image.getpixel((x,y))
                bgr_data.extend([b,g,r])
        arr_type = ctypes.c_char * len(bgr_data)
        arr_ctypes = arr_type(*bgr_data)

        return self.transport.setBackgroundImg(ctypes.cast(arr_ctypes, ctypes.POINTER(ctypes.c_ubyte)),width * height * 3)

    # 设置设备的按键图标 85 * 85
    def set_key_image(self, key, path):
        try:
            if isinstance(key, int):
                if key not in range(1, 19):
                    print(f"key '{key}' out of range. you should set (1 ~ 18)")
                    return -1
                logical_key = ButtonKey(key)
            else:
                logical_key = key

            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1

            # 获取硬件键值
            hardware_key = self.get_image_key(logical_key)

            image = Image.open(path)
            if hardware_key in range(1, 16):
                # icon
                rotated_image = to_native_key_format(self, image)
            elif hardware_key in range(16, 19):
                # second screen
                rotated_image = to_native_seondscreen_format(self, image)
            rotated_image.save("Temporary.jpg", "JPEG", subsampling=0, quality=100)
            returnvalue = self.transport.setKeyImg(bytes("Temporary.jpg",'utf-8'), hardware_key)
            os.remove("Temporary.jpg")
            return returnvalue

        except Exception as e:
            print(f"Error: {e}")
            return -1

    def get_serial_number(self, lenth):
        return self.serial_number

    def key_image_format(self):
        return {
            'size': (85, 85),
            'format': "JPEG",
            'rotation': 90,
            'flip': (False, False)
        }

    def secondscreen_image_format(self):
        return {
            'size': (80, 80),
            'format': "JPEG",
            'rotation': 90,
            'flip': (False, False)
        }

    def touchscreen_image_format(self):
        return {
            'size': (854, 480),
            'format': "JPEG",
            'rotation': 0,
            'flip': (True, False)
        }

    # 设置设备参数
    def set_device(self):
        self.transport.set_report_size(513, 513, 0)
        self.feature_option.deviceType = device_type.dock_293s
        pass
