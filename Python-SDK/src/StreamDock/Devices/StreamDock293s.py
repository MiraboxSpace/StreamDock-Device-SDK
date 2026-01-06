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
    """StreamDock293s device class - supports 15 keys and 3 secondary screen keys"""

    KEY_COUNT = 18
    KEY_MAP = False

    # Image key mapping: logical key -> hardware key (for setting images)
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
        # Secondary screen keys 16-18
        ButtonKey.KEY_16: 16,
        ButtonKey.KEY_17: 17,
        ButtonKey.KEY_18: 18,
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
        raise ValueError(f"StreamDock293s: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        Decode hardware event codes into a unified InputEvent

        The 293s device supports only regular buttons; hardware code range 1-18
        """
        # Handle state value: 0x02=release, 0x01=press
        normalized_state = 1 if state == 0x01 else 0

        # Regular button events (1-18)
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state
            )

        # Unknown event
        return InputEvent(event_type=EventType.UNKNOWN)

    # Set device screen brightness
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)

    # Set device background image 854 * 480
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

    # Set device key icon image 85 * 85
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

    def get_serial_number(self):
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

    # Set device parameters
    def set_device(self):
        self.transport.set_report_size(513, 513, 0)
        self.feature_option.deviceType = device_type.dock_293s
        pass
