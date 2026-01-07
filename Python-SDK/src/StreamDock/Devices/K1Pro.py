from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import InputEvent, ButtonKey, EventType, KnobId, Direction
from PIL import Image
import ctypes
import ctypes.util
import os, io
from ..ImageHelpers.PILHelper import *
import random


class K1Pro(StreamDock):
    """K1Pro device class - supports 6 keys and 3 knobs"""

    KEY_COUNT = 6
    KEY_MAP = False

    # Image key mapping: logical key -> hardware key (for setting images)
    _IMAGE_KEY_MAP = {
        ButtonKey.KEY_1: 0x05,
        ButtonKey.KEY_2: 0x03,
        ButtonKey.KEY_3: 0x01,
        ButtonKey.KEY_4: 0x06,
        ButtonKey.KEY_5: 0x04,
        ButtonKey.KEY_6: 0x02,
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
        raise ValueError(f"K1Pro: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        Decode hardware event codes into a unified InputEvent

        Hardware code mapping:
        - Keys: 0x05, 0x03, 0x01, 0x06, 0x04, 0x02
        - Knob 1 press: 0x25
        - Knob 2 press: 0x30
        - Knob 3 press: 0x31
        - Knob 1 rotation: 0x50 (left), 0x51 (right)
        - Knob 2 rotation: 0x60 (left), 0x61 (right)
        - Knob 3 rotation: 0x90 (left), 0x91 (right)
        """
        # Handle state value: 0x02=release, 0x01=press
        normalized_state = 1 if state == 0x01 else 0

        # Regular button events
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state,
            )

        # Knob press event
        knob_press_map = {
            0x25: KnobId.KNOB_1,
            0x30: KnobId.KNOB_2,
            0x31: KnobId.KNOB_3,
        }
        if hardware_code in knob_press_map:
            return InputEvent(
                event_type=EventType.KNOB_PRESS,
                knob_id=knob_press_map[hardware_code],
                state=normalized_state,
            )

        # Knob rotation event
        knob_rotate_map = {
            0x50: (KnobId.KNOB_1, Direction.LEFT),
            0x51: (KnobId.KNOB_1, Direction.RIGHT),
            0x60: (KnobId.KNOB_2, Direction.LEFT),
            0x61: (KnobId.KNOB_2, Direction.RIGHT),
            0x90: (KnobId.KNOB_3, Direction.LEFT),
            0x91: (KnobId.KNOB_3, Direction.RIGHT),
        }
        if hardware_code in knob_rotate_map:
            knob_id, direction = knob_rotate_map[hardware_code]
            return InputEvent(
                event_type=EventType.KNOB_ROTATE, knob_id=knob_id, direction=direction
            )

        # Unknown event
        return InputEvent(event_type=EventType.UNKNOWN)

    # Set device screen brightness
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)

    def set_touchscreen_image(self, path):
        """Background setting not supported"""
        return 0

    # Set device key icon image 64 * 64
    def set_key_image(self, key, path):
        try:
            if isinstance(key, int):
                if key not in range(1, 7):
                    print(f"key '{key}' out of range. you should set (1 ~ 6)")
                    return -1
                logical_key = ButtonKey(key)
            else:
                logical_key = key

            if not os.path.exists(path):
                print(f"Error: The image file '{path}' does not exist.")
                return -1

            # Get hardware key value
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

    # TODO
    def set_key_imageData(self, key, path):
        pass

    # Get device serial number
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
            "size": (800, 480),
            "format": "JPEG",
            "rotation": 180,
            "flip": (False, False),
        }

    # Set device parameters
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.transport.set_report_id(0x04)
        self.feature_option.deviceType = device_type.k1pro
        pass

    def set_keyboard_backlight_brightness(self, brightness):
        """
        Set the keyboard backlight brightness.

        Args:
            brightness: Brightness value (0-6)
        """
        self.transport.set_keyboard_backlight_brightness(brightness)

    def set_keyboard_lighting_effects(self, effect: int):
        """
        Set the keyboard lighting effect.
        0 is static lighting.
        Args:
            effect: Effect mode identifier (0-9)
        """
        if(effect==0):
            self.set_keyboard_lighting_speed(0)
        self.transport.set_keyboard_lighting_effects(effect)

    def set_keyboard_lighting_speed(self, speed: int):
        """
        Set the keyboard lighting effect speed.
        Args:
            speed: Speed value for lighting effects (0-7)
        """
        self.transport.set_keyboard_lighting_speed(speed)

    def set_keyboard_rgb_backlight(self, red: int, green: int, blue: int):
        """
        Set the keyboard RGB backlight color.

        Args:
            red: Red component (0-255)
            green: Green component (0-255)
            blue: Blue component (0-255)
        """
        self.transport.set_keyboard_rgb_backlight(red, green, blue)

    def keyboard_os_mode_switch(self, os_mode: int):
        """
        Set the keyboard OS mode.

        Args:
            os_mode: OS mode identifier
        """
        self.transport.keyboard_os_mode_switch(os_mode)
