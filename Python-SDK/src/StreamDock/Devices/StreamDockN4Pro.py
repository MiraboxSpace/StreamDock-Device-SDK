from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import InputEvent, ButtonKey, EventType, KnobId, Direction
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random


class StreamDockN4Pro(StreamDock):
    """StreamDockN4Pro 设备类 - 支持15个按键、4个旋钮和滑动手势"""

    KEY_COUNT = 15
    KEY_MAP = False

    # 图片键映射：逻辑键 -> 硬件键（用于设置图片）
    _IMAGE_KEY_MAP = {
        # 主按键 1-10
        ButtonKey.KEY_1: 11,
        ButtonKey.KEY_2: 12,
        ButtonKey.KEY_3: 13,
        ButtonKey.KEY_4: 14,
        ButtonKey.KEY_5: 15,
        ButtonKey.KEY_6: 6,
        ButtonKey.KEY_7: 7,
        ButtonKey.KEY_8: 8,
        ButtonKey.KEY_9: 9,
        ButtonKey.KEY_10: 10,
        # 副屏按键 11-14 (176x112)
        ButtonKey.KEY_11: 1,
        ButtonKey.KEY_12: 2,
        ButtonKey.KEY_13: 3,
        ButtonKey.KEY_14: 4,
        ButtonKey.KEY_15: 5,
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
        raise ValueError(f"StreamDockN4Pro: 不支持的按键 {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        将硬件事件码解码为统一的 InputEvent

        硬件码映射：
        - 按键: 1-15
        - 副屏按键: 0x40-0x43
        - 旋钮旋转: 0xA0, 0xA1(Knob1), 0x50, 0x51(Knob2), 0x90, 0x91(Knob3), 0x70, 0x71(Knob4)
        - 旋钮按下: 0x37(Knob1), 0x35(Knob2), 0x33(Knob3), 0x36(Knob4)
        - 滑动: 0x38(left), 0x39(right)
        """
        # 处理状态值：0x02=释放, 0x01=按下
        normalized_state = 1 if state == 0x01 else 0

        # 普通按键事件 (1-15)
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state
            )

        # 副屏按键事件
        secondary_key_map = {
            0x40: ButtonKey.KEY_11,
            0x41: ButtonKey.KEY_12,
            0x42: ButtonKey.KEY_13,
            0x43: ButtonKey.KEY_14,
        }
        if hardware_code in secondary_key_map:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=secondary_key_map[hardware_code],
                state=normalized_state
            )

        # 旋钮旋转事件
        knob_rotate_map = {
            0xA0: (KnobId.KNOB_1, Direction.LEFT),
            0xA1: (KnobId.KNOB_1, Direction.RIGHT),
            0x50: (KnobId.KNOB_2, Direction.LEFT),
            0x51: (KnobId.KNOB_2, Direction.RIGHT),
            0x90: (KnobId.KNOB_3, Direction.LEFT),
            0x91: (KnobId.KNOB_3, Direction.RIGHT),
            0x70: (KnobId.KNOB_4, Direction.LEFT),
            0x71: (KnobId.KNOB_4, Direction.RIGHT),
        }
        if hardware_code in knob_rotate_map:
            knob_id, direction = knob_rotate_map[hardware_code]
            return InputEvent(
                event_type=EventType.KNOB_ROTATE,
                knob_id=knob_id,
                direction=direction
            )

        # 旋钮按下事件
        knob_press_map = {
            0x37: KnobId.KNOB_1,
            0x35: KnobId.KNOB_2,
            0x33: KnobId.KNOB_3,
            0x36: KnobId.KNOB_4,
        }
        if hardware_code in knob_press_map:
            return InputEvent(
                event_type=EventType.KNOB_PRESS,
                knob_id=knob_press_map[hardware_code],
                state=normalized_state
            )

        # 滑动手势
        if hardware_code == 0x38:
            return InputEvent(event_type=EventType.SWIPE, direction=Direction.LEFT)
        if hardware_code == 0x39:
            return InputEvent(event_type=EventType.SWIPE, direction=Direction.RIGHT)

        # 未知事件
        return InputEvent(event_type=EventType.UNKNOWN)

    # 设置设备的屏幕亮度
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)

    # 设置设备的背景图片 800 * 480
    def set_touchscreen_image(self, path):
        try:
            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1

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

    # 设置设备的按键图标 112 * 112
    def set_key_image(self, key, path):
        try:
            if isinstance(key, int):
                if key not in range(1, 16):
                    print(f"key '{key}' out of range. you should set (1 ~ 15)")
                    return -1
                logical_key = ButtonKey(key)
            else:
                logical_key = key

            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1

            # 副屏按键使用不同的图片格式
            if logical_key.value in range(11, 15):
                return self.set_seondscreen_image(logical_key.value, path)

            # 获取硬件键值
            hardware_key = self.get_image_key(logical_key)

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
            res = self.transport.setKeyImgDualDevice(c_path, hardware_key)
            os.remove(temp_image_path)
            return res

        except Exception as e:
            print(f"Error: {e}")
            return -1

    # 设置设备的副屏按键图标 176 * 112
    def set_seondscreen_image(self, key, path):
        try:
            if key not in range(11, 15):
                print(f"key '{key}' out of range. you should set (11 ~ 14)")
                return -1

            logical_key = ButtonKey(key)
            hardware_key = self.get_image_key(logical_key)

            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1

            # open formatter
            image = Image.open(path)
            image = to_native_seondscreen_format(self, image)
            temp_image_path = (
                "rotated_key_image_" + str(random.randint(9999, 999999)) + ".jpg"
            )
            image.save(temp_image_path)

            # encode send
            path_bytes = temp_image_path.encode("utf-8")
            c_path = ctypes.c_char_p(path_bytes)
            res = self.transport.setKeyImgDualDevice(c_path, hardware_key)
            os.remove(temp_image_path)
            return res

        except Exception as e:
            print(f"Error: {e}")
            return -1

    # 待补充
    def set_key_imageData(self, key, path):
        pass

    # 获取设备的序列号
    def get_serial_number(self):
        return self.serial_number

    def key_image_format(self):
        return {
            "size": (112, 112),
            "format": "JPEG",
            "rotation": 180,
            "flip": (False, False),
        }

    def secondscreen_image_format(self):
        return {
            "size": (176, 112),
            "format": "JPEG",
            "rotation": 180,
            "flip": (False, False),
        }

    def touchscreen_image_format(self):
        return {
            "size": (800, 480),
            "format": "JPEG",
            "rotation": 180,
            "flip": (False, False),
        }

    # 设置设备参数
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.hasRGBLed = True
        self.feature_option.ledCounts = 4
        self.feature_option.deviceType = device_type.dock_n4pro
        pass
