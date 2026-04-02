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
from enum import Enum


def extract_last_number(code):
    """
    Extract consecutive digits after the last dot in a string and convert to int

    Args:
        code: String like "N3.02.013" or "N3.02.013V2"

    Returns:
        int: Extracted number, or None if not found
    """
    # Find the position of the last dot
    last_dot = code.rfind(".")
    if last_dot == -1:
        return None

    # Extract consecutive digits after the last dot
    num_str = ""
    for char in code[last_dot + 1 :]:
        if char.isdigit():
            num_str += char
        else:
            # Stop at the first non-digit character
            break

    # If digits were found, convert to int
    if num_str:
        return int(num_str)
    else:
        return None


class StreamDockN1(StreamDock):
    """StreamDockN1 device class - supports 20 inputs (15 main screen + 3 secondary screen + knob)"""

    KEY_COUNT = 20
    KEY_MAP = False

    # N1 device keys map directly; no mapping needed
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
        ButtonKey.KEY_16: 0x1E,
        ButtonKey.KEY_17: 0x1F,
    }

    class DeviceMode(Enum):
        KEYBOARD = 0
        CALCULATOR = 1
        DOCK = 2

    class SkinMode(Enum):
        KEYBOARD = 0x11
        KEYBOARD_LOCK = 0x1F
        CALCULATOR = 0xFF

    class SkinStatus(Enum):
        PRESS = 0
        RELEASE = 1

    # Reverse mapping: hardware key -> logical key (for event decoding)
    _HW_TO_LOGICAL_KEY = {v: k for k, v in _IMAGE_KEY_MAP.items()}

    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)
        self.devInfo = devInfo

    def open(self):
        super().open()
        self.transport.switchMode(2)

    def get_image_key(self, logical_key: ButtonKey) -> int:
        """
        Convert logical key value to hardware key value (for setting images)

        N1 device keys map directly

        Args:
            logical_key: Logical key enum

        Returns:
            int: Hardware key value
        """
        if logical_key in self._IMAGE_KEY_MAP:
            return self._IMAGE_KEY_MAP[logical_key]
        raise ValueError(f"StreamDockN1: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        Decode hardware event codes into a unified InputEvent

        The N1 device supports regular button and knob events:
        - Regular buttons 1-17: hardware codes 0x01-0x0F, 0x1E-0x1F
        - Knob press 18: hardware code 0x23
        - Knob rotation 19-20: hardware codes 0x32 (left), 0x33 (right)
        """

        # Knob rotation event
        knob_rotate_map = {
            0x32: (KnobId.KNOB_1, Direction.LEFT),
            0x33: (KnobId.KNOB_1, Direction.RIGHT),
        }
        if hardware_code in knob_rotate_map:
            knob_id, direction = knob_rotate_map[hardware_code]
            return InputEvent(
                event_type=EventType.KNOB_ROTATE, knob_id=knob_id, direction=direction
            )

        # Handle state value: 0x02=release, 0x01=press
        normalized_state = 1 if state == 0x01 else 0
        # Knob press event
        knob_press_map = {
            0x23: KnobId.KNOB_1,
        }
        if hardware_code in knob_press_map:
            return InputEvent(
                event_type=EventType.KNOB_PRESS,
                knob_id=knob_press_map[hardware_code],
                state=normalized_state,
            )

        # Regular button events (1-17)
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

    # Set device background image 480 * 854
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

    # Set device key icon image 96 * 96
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

            # N1 device keys map directly
            hardware_key = logical_key.value

            image = Image.open(path)
            if hardware_key in range(1, 16):
                # icon
                rotated_image = to_native_key_format(self, image)
            elif hardware_key in range(16, 19):
                # second screen
                rotated_image = to_native_seondscreen_format(self, image)
            else:
                print(f"Error: Invalid hardware key '{hardware_key}'.")
                return -1
            rotated_image.save("Temporary.jpg", "JPEG", subsampling=0, quality=90)
            returnvalue = self.transport.setKeyImgDualDevice(
                bytes("Temporary.jpg", "utf-8"), hardware_key
            )
            os.remove("Temporary.jpg")
            return returnvalue

        except Exception as e:
            print(f"Error: {e}")
            return -1

    def get_serial_number(self):
        return self.serial_number

    def switch_mode(self, mode: DeviceMode):
        # 0:calculator, 1:dock
        return self.transport.switchMode(mode.value)

    def change_page(self, page):
        return self.transport.changePage(page)

    def set_n1_skin_bitmap(
        self,
        path,
        skin_mode: SkinMode,
        skin_page: int,
        skin_status: SkinStatus,
        key_index: int,
    ):
        try:
            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1
            if self.SkinMode.CALCULATOR == skin_mode:
                if key_index < 1 or key_index > 18:
                    print(
                        f"Error: For CALCULATOR skin mode, key_index should be in range 1-18."
                    )
                    return -1
            elif (
                self.SkinMode.KEYBOARD == skin_mode
                or self.SkinMode.KEYBOARD_LOCK == skin_mode
            ):
                if key_index < 1 or key_index > 15:
                    print(
                        f"Error: For KEYBOARD skin mode, key_index should be in range 1-15."
                    )
                    return -1
            if skin_page < 1 or skin_page > 5:
                print(f"Error: skin_page should be in range 1-5.")
                return -1
            image = Image.open(path)
            if self.SkinMode.KEYBOARD == skin_mode and key_index in range(16, 19):
                image = to_native_seondscreen_format(self, image)
            else:
                image = to_native_key_format(self, image)
            temp_image_path = (
                "rotated_n1_skin_image_" + str(random.randint(9999, 999999)) + ".png"
            )
            image.save(temp_image_path)

            # encode send
            path_bytes = temp_image_path.encode("utf-8")
            c_path = ctypes.c_char_p(path_bytes)
            res = self.transport.setN1SkinBitMap(
                c_path, skin_mode.value, skin_page, skin_status.value, key_index
            )
            os.remove(temp_image_path)
            return res
        except Exception as e:
            print(f"Error: {e}")
            return -1

    def key_image_format(self):
        return {
            "size": (96, 96),
            "format": "JPEG",
            "rotation": 0,
            "flip": (False, False),
        }

    def secondscreen_image_format(self):
        return {
            "size": (80, 80),
            "format": "JPEG",
            "rotation": 0,
            "flip": (False, False),
        }

    def touchscreen_image_format(self):
        return {
            "size": (480, 854),
            "format": "JPEG",
            "rotation": 0,
            "flip": (False, False),
        }

    # Set device parameters
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.deviceType = device_type.dock_n1
        pass
