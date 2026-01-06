from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import InputEvent, ButtonKey, EventType, KnobId, Direction
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random


class StreamDockXL(StreamDock):
    """StreamDockXL device class - supports 36 inputs (32 keys + 2 knobs)"""

    KEY_COUNT = 36
    KEY_MAP = False

    # Image key mapping: logical key -> hardware key (for setting images)
    _IMAGE_KEY_MAP = {
        ButtonKey.KEY_1: 25,
        ButtonKey.KEY_2: 26,
        ButtonKey.KEY_3: 27,
        ButtonKey.KEY_4: 28,
        ButtonKey.KEY_5: 29,
        ButtonKey.KEY_6: 30,
        ButtonKey.KEY_7: 31,
        ButtonKey.KEY_8: 32,
        ButtonKey.KEY_9: 17,
        ButtonKey.KEY_10: 18,
        ButtonKey.KEY_11: 19,
        ButtonKey.KEY_12: 20,
        ButtonKey.KEY_13: 21,
        ButtonKey.KEY_14: 22,
        ButtonKey.KEY_15: 23,
        ButtonKey.KEY_16: 24,
        ButtonKey.KEY_17: 9,
        ButtonKey.KEY_18: 10,
        ButtonKey.KEY_19: 11,
        ButtonKey.KEY_20: 12,
        ButtonKey.KEY_21: 13,
        ButtonKey.KEY_22: 14,
        ButtonKey.KEY_23: 15,
        ButtonKey.KEY_24: 16,
        ButtonKey.KEY_25: 1,
        ButtonKey.KEY_26: 2,
        ButtonKey.KEY_27: 3,
        ButtonKey.KEY_28: 4,
        ButtonKey.KEY_29: 5,
        ButtonKey.KEY_30: 6,
        ButtonKey.KEY_31: 7,
        ButtonKey.KEY_32: 8,
    }

    # Reverse mapping: hardware key -> logical key (for event decoding)
    _HW_TO_LOGICAL_KEY = {v: k for k, v in _IMAGE_KEY_MAP.items()}

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
        raise ValueError(f"StreamDockXL: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        Decode hardware event codes into a unified InputEvent

        XL supports regular button and knob events:
        - Regular buttons 1-32: hardware codes 0x19-0x08
        - Left knob up/down 33-34: hardware codes 0x21 (up), 0x23 (down)
        - Right knob up/down 35-36: hardware codes 0x24 (up), 0x26 (down)
        """

        knob_rotate_map = {
            0x23: (KnobId.KNOB_1, Direction.LEFT),
            0x21: (KnobId.KNOB_1, Direction.RIGHT),
            0x24: (KnobId.KNOB_2, Direction.LEFT),
            0x26: (KnobId.KNOB_2, Direction.RIGHT),
        }

        # Knob rotation event
        if hardware_code in knob_rotate_map:
            knob_id, direction = knob_rotate_map[hardware_code]
            return InputEvent(
                event_type=EventType.KNOB_ROTATE, knob_id=knob_id, direction=direction
            )
        # Handle state value: 0x02=release, 0x01=press
        normalized_state = 1 if state == 0x01 else 0

        # Regular button events (1-32)
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state,
            )

        # Unknown event
        return InputEvent(event_type=EventType.UNKNOWN)

    # Set device screen brightness
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)

    # Set device background image 1024 * 600
    def set_touchscreen_image(self, path):
        try:
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

    # Set device key icon image 80 * 80
    def set_key_image(self, key, path):
        try:
            if isinstance(key, int):
                if key not in range(1, 33):
                    print(f"key '{key}' out of range. you should set (1 ~ 32)")
                    return -1
                logical_key = ButtonKey(key)
            else:
                logical_key = key

            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1

            # Get hardware key value
            hardware_key = self.get_image_key(logical_key)

            # XL supports setting icons only for keys 1-32 (knob events do not require icons)
            if hardware_key not in range(1, 33):
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

    # Set device parameters
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.hasRGBLed = True
        self.feature_option.ledCounts = 6
        self.feature_option.deviceType = device_type.dock_xl
        pass
