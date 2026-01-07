from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import InputEvent, ButtonKey, EventType, KnobId, Direction
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random


class StreamDockN3(StreamDock):
    """StreamDockN3 device class - supports 18 inputs (6 main keys + 3 bottom buttons + 3 knobs)"""

    KEY_COUNT = 18
    KEY_MAP = False

    # N3 device keys map directly
    _IMAGE_KEY_MAP = {
        ButtonKey.KEY_1: 1,
        ButtonKey.KEY_2: 2,
        ButtonKey.KEY_3: 3,
        ButtonKey.KEY_4: 4,
        ButtonKey.KEY_5: 5,
        ButtonKey.KEY_6: 6,
        ButtonKey.KEY_7: 0x25,
        ButtonKey.KEY_8: 0x30,
        ButtonKey.KEY_9: 0x31,
    }

    # Reverse mapping: hardware key -> logical key (for event decoding)
    _HW_TO_LOGICAL_KEY = {v: k for k, v in _IMAGE_KEY_MAP.items()}

    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)

    def get_image_key(self, logical_key: ButtonKey) -> int:
        """
        Convert logical key value to hardware key value (for setting images)

        N3 device keys map directly

        Args:
            logical_key: Logical key enum

        Returns:
            int: Hardware key value
        """
        if logical_key in self._IMAGE_KEY_MAP:
            return self._IMAGE_KEY_MAP[logical_key]
        raise ValueError(f"StreamDockN3: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        Decode hardware event codes into a unified InputEvent

        The N3 device supports regular button and knob events:
        - Regular buttons 1-9: hardware codes 0x01-0x06, 0x25, 0x30, 0x31
        - Knob press 10-12: hardware codes 0x33 (bottom-left), 0x34 (bottom-right), 0x35 (top)
        - Knob rotation 13-18: hardware codes 0x90/0x91 (bottom-left), 0x60/0x61 (bottom-right), 0x50/0x51 (top)
        """

        # Regular button events (1-9)
        # Handle state value: 0x02=release, 0x01=press
        normalized_state = 1 if state == 0x01 else 0
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state,
            )

        # Knob rotation event
        knob_rotate_map = {
            0x90: (KnobId.KNOB_1, Direction.LEFT),
            0x91: (KnobId.KNOB_1, Direction.RIGHT),
            0x60: (KnobId.KNOB_2, Direction.LEFT),
            0x61: (KnobId.KNOB_2, Direction.RIGHT),
            0x50: (KnobId.KNOB_3, Direction.LEFT),
            0x51: (KnobId.KNOB_3, Direction.RIGHT),
        }
        if hardware_code in knob_rotate_map:
            knob_id, direction = knob_rotate_map[hardware_code]
            return InputEvent(
                event_type=EventType.KNOB_ROTATE, knob_id=knob_id, direction=direction
            )
        # Knob press event
        knob_press_map = {
            0x33: KnobId.KNOB_1,
            0x34: KnobId.KNOB_2,
            0x35: KnobId.KNOB_3,
        }
        if hardware_code in knob_press_map:
            return InputEvent(
                event_type=EventType.KNOB_PRESS,
                knob_id=knob_press_map[hardware_code],
                state=normalized_state,
            )

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

    # Set device key icon image 112 * 112
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

            # Get hardware key value
            hardware_key = self.get_image_key(logical_key)

            # N3 supports setting icons only for keys 1-9 (knob events do not require icons)
            if hardware_key not in range(1, 10):
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
            "size": (64, 64),
            "format": "JPEG",
            "rotation": -90,
            "flip": (False, False),
        }

    def touchscreen_image_format(self):
        return {
            "size": (320, 240),
            "format": "JPEG",
            "rotation": -90,
            "flip": (False, False),
        }

    # Set device parameters
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.deviceType = device_type.dock_n3
        pass
