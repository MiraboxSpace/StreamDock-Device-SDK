from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import InputEvent, ButtonKey, EventType, KnobId, Direction


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
    }

    # Input codes are separate from image-slot numbers.
    _HW_TO_LOGICAL_KEY = {
        0x01: ButtonKey.KEY_1,
        0x02: ButtonKey.KEY_2,
        0x03: ButtonKey.KEY_3,
        0x04: ButtonKey.KEY_4,
        0x05: ButtonKey.KEY_5,
        0x06: ButtonKey.KEY_6,
        0x25: ButtonKey.KEY_7,
        0x30: ButtonKey.KEY_8,
        0x31: ButtonKey.KEY_9,
    }

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
        # State values: 0x00=release, 0x01=press
        if state not in (0, 1):
            return InputEvent(event_type=EventType.UNKNOWN)
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
            if state != 0:
                return InputEvent(event_type=EventType.UNKNOWN)
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

    # Set the boot logo using the model's input canvas.
    def set_touchscreen_image(self, path):
        return self._set_boot_logo(path)

    # Set an image on a valid display slot.
    def set_key_image(self, key, path):
        return self._set_key_image(key, path)

    # TODO
    def set_key_imageData(self, key, path):
        pass

    # Get device firmware version
    def get_serial_number(self):
        return self.serial_number

    def key_image_format(self, key=None):
        legacy = self.vendor_id == 0x6602
        return {
            "size": (60, 60) if legacy else (64, 64),
            "format": "JPEG",
            "rotation": 0 if legacy else -90,
            "flip": (False, False),
        }

    def touchscreen_image_format(self):
        legacy = self.vendor_id == 0x6602
        return {
            "size": (320, 240),
            "format": "RAW" if legacy else "JPEG",
            "pixel_format": "RGB565",
            "rotation": -90,
            "flip": (False, False),
        }

    # Set device parameters
    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.deviceType = device_type.dock_n3
        self._configure_firmware()

    def _configure_firmware(self):
        self.feature_option.supportKeyGif = (
            self.vendor_id != 0x6602 and "V25.N3" not in self.firmware_version.upper()
        )
