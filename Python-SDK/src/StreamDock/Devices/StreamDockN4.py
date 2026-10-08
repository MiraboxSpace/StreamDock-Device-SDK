from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import InputEvent, ButtonKey, EventType, KnobId, Direction
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random


class StreamDockN4(StreamDock):
    """StreamDockN4 device class - supports 14 keys, 4 knobs, and swipe gestures."""

    KEY_COUNT = 14
    DIAL_COUNT = 4
    KEY_MAP = False

    # Image key mapping: logical key -> hardware key (for setting images)
    _IMAGE_KEY_MAP = {
        ButtonKey.KEY_1: 1,
        ButtonKey.KEY_2: 2,
        ButtonKey.KEY_3: 3,
        ButtonKey.KEY_4: 4,
        ButtonKey.KEY_5: 6,
        ButtonKey.KEY_6: 7,
        ButtonKey.KEY_7: 8,
        ButtonKey.KEY_8: 9,
        ButtonKey.KEY_9: 10,
        ButtonKey.KEY_10: 11,
        ButtonKey.KEY_11: 12,
        ButtonKey.KEY_12: 13,
        ButtonKey.KEY_13: 14,
        ButtonKey.KEY_14: 15,
    }

    # Input codes differ from the hardware values used to write images.
    _HW_TO_LOGICAL_KEY = {
        0x40: ButtonKey.KEY_1,
        0x41: ButtonKey.KEY_2,
        0x42: ButtonKey.KEY_3,
        0x43: ButtonKey.KEY_4,
        6: ButtonKey.KEY_5,
        7: ButtonKey.KEY_6,
        8: ButtonKey.KEY_7,
        9: ButtonKey.KEY_8,
        10: ButtonKey.KEY_9,
        1: ButtonKey.KEY_10,
        2: ButtonKey.KEY_11,
        3: ButtonKey.KEY_12,
        4: ButtonKey.KEY_13,
        5: ButtonKey.KEY_14,
    }

    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)

    def get_image_key(self, logical_key: ButtonKey) -> int:
        """
        Convert logical key value to hardware key value (for setting images)

        Args:
            logical_key: Logical key enum

        Returns:
            int: Hardware key value
        """
        if logical_key in self._IMAGE_KEY_MAP:
            return self._IMAGE_KEY_MAP[logical_key]
        raise ValueError(f"StreamDockN4: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        Decode hardware event codes into a unified InputEvent

        Supports LCD/main buttons, knob rotation/press, and swipe gestures.
        """
        # 0x01=press; 0x00=release (other values also normalize to release).
        normalized_state = 1 if state == 0x01 else 0

        # LCD keys 1-4 and main screen keys 5-14
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state
            )

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
                direction=direction,
            )

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
                state=normalized_state,
            )

        if hardware_code == 0x38:
            return InputEvent(event_type=EventType.SWIPE, direction=Direction.LEFT)
        if hardware_code == 0x39:
            return InputEvent(event_type=EventType.SWIPE, direction=Direction.RIGHT)

        # Unknown event
        return InputEvent(event_type=EventType.UNKNOWN)

    # Set device screen brightness
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)

    # Set device background image 800 * 480
    def set_touchscreen_image(self, path):
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

    # Set device key icon image 112 * 112
    def set_key_image(self, key, path):
        try:
            if isinstance(key, int):
                if key not in range(1, 15):
                    print(f"key '{key}' out of range. you should set (1 ~ 14)")
                    return -1
                logical_key = ButtonKey(key)
            else:
                logical_key = key

            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1

            # Secondary screen keys 1-4
            if logical_key.value in range(1, 5):
                return self.set_seondscreen_image(logical_key.value, path)

            # Get hardware key value
            hardware_key = self.get_image_key(logical_key)

            # open formatter
            image = Image.open(path)
            image = to_native_key_format(self, image, logical_key)
            temp_image_path = "rotated_key_image_" + str(random.randint(9999, 999999)) + ".jpg"
            image.save(temp_image_path)

            # encode send
            path_bytes = temp_image_path.encode('utf-8')
            c_path = ctypes.c_char_p(path_bytes)
            res = self.transport.setKeyImgDualDevice(c_path, hardware_key)
            os.remove(temp_image_path)
            return res

        except Exception as e:
            print(f"Error: {e}")
            return -1

    # Set device secondary screen key icon image 176 * 112
    def set_seondscreen_image(self, key, path):
        try:
            if key not in range(1, 5):
                print(f"key '{key}' out of range. you should set (1 ~ 4)")
                return -1

            logical_key = ButtonKey(key)
            hardware_key = self.get_image_key(logical_key)

            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1

            # open formatter
            image = Image.open(path)
            image = to_native_key_format(self, image, logical_key)
            temp_image_path = "rotated_key_image_" + str(random.randint(9999, 999999)) + ".jpg"
            image.save(temp_image_path)

            # encode send
            path_bytes = temp_image_path.encode('utf-8')
            c_path = ctypes.c_char_p(path_bytes)
            res = self.transport.setKeyImgDualDevice(c_path, hardware_key)
            os.remove(temp_image_path)
            return res

        except Exception as e:
            print(f"Error: {e}")
            return -1

    # TODO
    def set_key_imageData(self, key, path):
        pass

    # Get device firmware version
    def get_serial_number(self):
        return self.serial_number

    def key_image_format(self, key=None):
        if key is not None and key in range(1, 5):
            return self.secondscreen_image_format()
        return {
            'size': (112, 112),
            'format': "JPEG",
            'rotation': 180,
            'flip': (False, False)
        }

    def secondscreen_image_format(self):
        return {
            'size': (176, 112),
            'format': "JPEG",
            'rotation': 180,
            'flip': (False, False)
        }

    def touchscreen_image_format(self):
        return {
            'size': (800, 480),
            'format': "JPEG",
            'rotation': 180,
            'flip': (True, False)
        }

    # Set device parameters
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.hasKnob = True
        self.feature_option.knobCounts = 4
        self.feature_option.deviceType = device_type.dock_n4
        pass
