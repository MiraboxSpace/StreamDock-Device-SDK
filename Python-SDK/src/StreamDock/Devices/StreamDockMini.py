from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import InputEvent, ButtonKey, EventType, DIPSwitchId, Direction
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random


class StreamDockMini(StreamDock):
    """StreamDockMini device class - supports 6 keys and 2 DIP switches"""

    KEY_COUNT = 6
    KEY_MAP = False

    # Image key mapping: logical key -> hardware key (for setting images)
    _IMAGE_KEY_MAP = {
        ButtonKey.KEY_1: 1,
        ButtonKey.KEY_2: 2,
        ButtonKey.KEY_3: 3,
        ButtonKey.KEY_4: 4,
        ButtonKey.KEY_5: 5,
        ButtonKey.KEY_6: 6
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
        raise ValueError(f"StreamDockMini: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        Decode hardware event codes into a unified InputEvent

        StreamDockMini supports regular button and DIP switch events:
        - Regular buttons 1-6: hardware codes 0x01-0x06
        - DIP 1: 0x24 (left), 0x26 (right), 0x25 (press)
        - DIP 2: 0x21 (left), 0x23 (right), 0x22 (press)
        State 0x01 means active/press, state 0x00 means end/release.
        """
        normalized_state = 1 if state == 0x01 else 0

        dip_switch_map = {
            0x24: (DIPSwitchId.DIP_1, Direction.LEFT),
            0x26: (DIPSwitchId.DIP_1, Direction.RIGHT),
            0x25: (DIPSwitchId.DIP_1, None),
            0x21: (DIPSwitchId.DIP_2, Direction.LEFT),
            0x23: (DIPSwitchId.DIP_2, Direction.RIGHT),
            0x22: (DIPSwitchId.DIP_2, None),
        }
        if hardware_code in dip_switch_map:
            dip_id, direction = dip_switch_map[hardware_code]
            return InputEvent(
                event_type=EventType.DIP_SWITCH,
                dip_id=dip_id,
                direction=direction,
                state=normalized_state,
            )

        # Regular button events (1-6)
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state,
            )

        # Unknown event
        return InputEvent(event_type=EventType.UNKNOWN)
    
    def set_frame_background(self, path):
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
            image.save(temp_image_path, quality=80)

            # encode send
            path_bytes = temp_image_path.encode("utf-8")
            c_path = ctypes.c_char_p(path_bytes)
            res = self.transport.setBackgroundImgFrame(
                c_path,
                1024,
                600,
            )
            os.remove(temp_image_path)
            return res
        except Exception as e:
            print(f"Error: {e}")
            return -1
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

    # Set device key icon image 80 * 80. PNG and JPEG input files are supported.
    def set_key_image(self, key, path):
        try:
            if isinstance(key, int):
                if key not in range(1, self.KEY_COUNT + 1):
                    print(
                        f"key '{key}' out of range. you should set (1 ~ {self.KEY_COUNT})"
                    )
                    return -1
                logical_key = ButtonKey(key)
            else:
                logical_key = key

            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1

            # Get hardware key value
            hardware_key = self.get_image_key(logical_key)

            # Mini supports setting icons only for keys 1-6.
            if hardware_key not in range(1, self.KEY_COUNT + 1):
                return -1

            # open formatter
            image = Image.open(path)
            image = to_native_key_format(self, image)
            temp_image_path = (
                "rotated_key_image_" + str(random.randint(9999, 999999)) + ".jpg"
            )
            image.save(temp_image_path, "JPEG", quality=80)

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
            "rotation": 90,
            "flip": (False, False),
        }

    def touchscreen_image_format(self):
        return {
            "size": (320, 240),
            "format": "JPEG",
            "rotation": 90,
            "flip": (False, False),
        }

    # Set device parameters
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.hasRGBLed = True
        self.feature_option.ledCounts = 12
        self.feature_option.deviceType = device_type.dock_mini
        pass
