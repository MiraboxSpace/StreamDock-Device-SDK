# -*- coding: utf-8 -*-
from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import InputEvent, ButtonKey, EventType, KnobId, Direction
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
    """StreamDockN1 设备类 - 支持20个输入（15主屏+3副屏+旋钮）"""

    KEY_COUNT = 20
    KEY_MAP = False

    # N1 设备按键直接对应，无需映射
    _IMAGE_KEY_MAP = {
        ButtonKey.KEY_1: 1,
        ButtonKey.KEY_2: 2,
        ButtonKey.KEY_3: 3,
        ButtonKey.KEY_4: 4,
        ButtonKey.KEY_5: 5,
        ButtonKey.KEY_6: 6,
        ButtonKey.KEY_7: 7,
        ButtonKey.KEY_8: 8,
        ButtonKey.KEY_9: 9,
        ButtonKey.KEY_10: 10,
        ButtonKey.KEY_11: 11,
        ButtonKey.KEY_12: 12,
        ButtonKey.KEY_13: 13,
        ButtonKey.KEY_14: 14,
        ButtonKey.KEY_15: 15,
        ButtonKey.KEY_16: 16,
        ButtonKey.KEY_17: 17,
        ButtonKey.KEY_18: 18,
    }

    # 反向映射：硬件键 -> 逻辑键（用于事件解码）
    _HW_TO_LOGICAL_KEY = {v: k for k, v in _IMAGE_KEY_MAP.items()}

    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)
        self.devInfo = devInfo

    def open(self):
        super().open()
        self.transport.switchMode(2)

    def get_image_key(self, logical_key: ButtonKey) -> int:
        """
        将逻辑键值转换为硬件键值（用于设置图片）

        N1 设备按键直接对应

        Args:
            logical_key: 逻辑键值枚举

        Returns:
            int: 硬件键值
        """
        if logical_key in self._IMAGE_KEY_MAP:
            return self._IMAGE_KEY_MAP[logical_key]
        raise ValueError(f"StreamDockN1: 不支持的按键 {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        将硬件事件码解码为统一的 InputEvent

        N1 设备支持普通按键和旋钮事件：
        - 普通按键 1-17: 硬件码 0x01-0x0F, 0x1E-0x1F
        - 旋钮按下 18: 硬件码 0x23
        - 旋钮旋转 19-20: 硬件码 0x32 (左), 0x33 (右)
        """

        # 旋钮旋转事件
        knob_rotate_map = {
            0x32: (KnobId.KNOB_1, Direction.LEFT),
            0x33: (KnobId.KNOB_1, Direction.RIGHT),
        }
        if hardware_code in knob_rotate_map:
            knob_id, direction = knob_rotate_map[hardware_code]
            return InputEvent(
                event_type=EventType.KNOB_ROTATE,
                knob_id=knob_id,
                direction=direction
            )
            
        # 处理状态值：0x02=释放, 0x01=按下
        normalized_state = 1 if state == 0x01 else 0
        # 旋钮按下事件
        knob_press_map = {
            0x23: KnobId.KNOB_1,
        }
        if hardware_code in knob_press_map:
            return InputEvent(
                event_type=EventType.KNOB_PRESS,
                knob_id=knob_press_map[hardware_code],
                state=normalized_state
            )

        # 普通按键事件 (1-17)
        if logical_key in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[logical_key],
                state=normalized_state
            )

        # 未知事件
        return InputEvent(event_type=EventType.UNKNOWN)

    # 设置设备的屏幕亮度
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)

    # 设置设备的背景图片 480 * 854
    def set_touchscreen_image(self, path):
        version_str = self.get_serial_number()
        version_num = extract_last_number(version_str)
        if version_num is None:
            return -1
        if version_num >= 13:
            try:
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

            # N1 设备按键直接对应
            hardware_key = logical_key.value

            image = Image.open(path)
            if hardware_key in range(1, 16):
                # icon
                rotated_image = to_native_key_format(self, image)
            else:
                return  -1  # N1 设备仅支持按键1-15设置图标
            rotated_image.save("Temporary.jpg", "JPEG", subsampling=0, quality=90)
            returnvalue = self.transport.setKeyImgDualDevice(bytes("Temporary.jpg",'utf-8'), hardware_key)
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
        self.feature_option.deviceType = device_type.dock_n1
        pass
