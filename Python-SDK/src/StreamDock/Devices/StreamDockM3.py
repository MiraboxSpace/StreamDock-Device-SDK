from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import Direction, InputEvent, ButtonKey, EventType, KnobId


class StreamDockM3(StreamDock):
    """StreamDockM3 device class - supports 15 keys"""

    KEY_COUNT = 15
    KEY_MAP = False

    # Image key mapping: logical key -> hardware key (for setting images)
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
    }

    # Input codes are separate from image-slot numbers.
    _HW_TO_LOGICAL_KEY = {
        0x01: ButtonKey.KEY_11,
        0x02: ButtonKey.KEY_12,
        0x03: ButtonKey.KEY_13,
        0x04: ButtonKey.KEY_14,
        0x05: ButtonKey.KEY_15,
        0x06: ButtonKey.KEY_6,
        0x07: ButtonKey.KEY_7,
        0x08: ButtonKey.KEY_8,
        0x09: ButtonKey.KEY_9,
        0x0A: ButtonKey.KEY_10,
        0x0B: ButtonKey.KEY_1,
        0x0C: ButtonKey.KEY_2,
        0x0D: ButtonKey.KEY_3,
        0x0E: ButtonKey.KEY_4,
        0x0F: ButtonKey.KEY_5,
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
        raise ValueError(f"StreamDockM3: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        Decode hardware event codes into a unified InputEvent

        M3 exposes 15 input keys and 3 knobs, numbered from bottom to top.
        """
        # State values: 0x00=release, 0x01=press
        if state not in (0, 1):
            return InputEvent(event_type=EventType.UNKNOWN)
        normalized_state = 1 if state == 0x01 else 0

        # Regular button events (1-15)
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state,
            )
        # Knob rotation event
        knob_rotate_map = {
            0x50: (KnobId.KNOB_3, Direction.LEFT),
            0x51: (KnobId.KNOB_3, Direction.RIGHT),
            0x90: (KnobId.KNOB_2, Direction.LEFT),
            0x91: (KnobId.KNOB_2, Direction.RIGHT),
            0xA0: (KnobId.KNOB_1, Direction.LEFT),
            0xA1: (KnobId.KNOB_1, Direction.RIGHT),
        }
        if hardware_code in knob_rotate_map:
            if state != 0:
                return InputEvent(event_type=EventType.UNKNOWN)
            knob_id, direction = knob_rotate_map[hardware_code]
            return InputEvent(
                event_type=EventType.KNOB_ROTATE, knob_id=knob_id, direction=direction
            )

        # Knob press event
        knob_press_map = {
            0x35: KnobId.KNOB_3,
            0x33: KnobId.KNOB_2,
            0x37: KnobId.KNOB_1,
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

    # Set the boot logo using the model's input canvas.
    def set_touchscreen_image(self, path):
        return self._set_boot_logo(path)

    # Set an image on a valid display slot.
    def set_key_image(self, key, path):
        return self._set_key_image(key, path)

    def set_frame_background(self, path):
        return self._set_frame_background(path)

    def set_key_imageData(self, key, path):
        pass

    def magnetic_calibration(self):
        """Perform magnetic calibration."""
        self.transport.magnetic_calibration()

    # Get device firmware version
    def get_serial_number(self):
        return self.serial_number

    def key_image_format(self, key=None):
        return {
            "size": (96, 96),
            "format": "PNG",
            "rotation": 90,
            "flip": (False, False),
        }

    def touchscreen_image_format(self):
        return {
            "size": (854, 480),
            "format": "JPEG",
            "rotation": 90,
            "flip": (False, False),
        }

    # Set device parameters
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.deviceType = device_type.dock_m3
        self.feature_option.supportBackgroundGif = True
        pass
