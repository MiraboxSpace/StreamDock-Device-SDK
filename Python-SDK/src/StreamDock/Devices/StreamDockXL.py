from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..DeviceConfig import StreamDockXLConfig
from ..InputTypes import InputEvent, ButtonKey, EventType, DIPSwitchId, Direction


class StreamDockXL(StreamDock):
    """StreamDockXL device class - supports 32 keys and 2 DIP switches."""

    KEY_COUNT = 32
    DIP_SWITCH_COUNT = 2
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
        ButtonKey.KEY_16: 16,
        ButtonKey.KEY_17: 17,
        ButtonKey.KEY_18: 18,
        ButtonKey.KEY_19: 19,
        ButtonKey.KEY_20: 20,
        ButtonKey.KEY_21: 21,
        ButtonKey.KEY_22: 22,
        ButtonKey.KEY_23: 23,
        ButtonKey.KEY_24: 24,
        ButtonKey.KEY_25: 25,
        ButtonKey.KEY_26: 26,
        ButtonKey.KEY_27: 27,
        ButtonKey.KEY_28: 28,
        ButtonKey.KEY_29: 29,
        ButtonKey.KEY_30: 30,
        ButtonKey.KEY_31: 31,
        ButtonKey.KEY_32: 32,
    }

    # Input codes are separate from image-slot numbers.
    _HW_TO_LOGICAL_KEY = {
        0x01: ButtonKey.KEY_25,
        0x02: ButtonKey.KEY_26,
        0x03: ButtonKey.KEY_27,
        0x04: ButtonKey.KEY_28,
        0x05: ButtonKey.KEY_29,
        0x06: ButtonKey.KEY_30,
        0x07: ButtonKey.KEY_31,
        0x08: ButtonKey.KEY_32,
        0x09: ButtonKey.KEY_17,
        0x0A: ButtonKey.KEY_18,
        0x0B: ButtonKey.KEY_19,
        0x0C: ButtonKey.KEY_20,
        0x0D: ButtonKey.KEY_21,
        0x0E: ButtonKey.KEY_22,
        0x0F: ButtonKey.KEY_23,
        0x10: ButtonKey.KEY_24,
        0x11: ButtonKey.KEY_9,
        0x12: ButtonKey.KEY_10,
        0x13: ButtonKey.KEY_11,
        0x14: ButtonKey.KEY_12,
        0x15: ButtonKey.KEY_13,
        0x16: ButtonKey.KEY_14,
        0x17: ButtonKey.KEY_15,
        0x18: ButtonKey.KEY_16,
        0x19: ButtonKey.KEY_1,
        0x1A: ButtonKey.KEY_2,
        0x1B: ButtonKey.KEY_3,
        0x1C: ButtonKey.KEY_4,
        0x1D: ButtonKey.KEY_5,
        0x1E: ButtonKey.KEY_6,
        0x1F: ButtonKey.KEY_7,
        0x20: ButtonKey.KEY_8,
    }

    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)
        self.config = StreamDockXLConfig()

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

        XL supports regular button and DIP switch events:
        - Regular buttons 1-32: hardware codes 0x19-0x08
        - DIP 1: 0x21 (left), 0x23 (right), 0x22 (center)
        - DIP 2: 0x24 (left), 0x26 (right), 0x25 (center)
        State 0x01 means active/press; any other state means end/release.
        """
        if state not in (0, 1):
            return InputEvent(event_type=EventType.UNKNOWN)
        normalized_state = 1 if state == 0x01 else 0

        dip_switch_map = {
            0x21: (DIPSwitchId.DIP_1, Direction.LEFT),
            0x23: (DIPSwitchId.DIP_1, Direction.RIGHT),
            0x22: (DIPSwitchId.DIP_1, None),
            0x24: (DIPSwitchId.DIP_2, Direction.LEFT),
            0x26: (DIPSwitchId.DIP_2, Direction.RIGHT),
            0x25: (DIPSwitchId.DIP_2, None),
        }

        if hardware_code in dip_switch_map:
            dip_id, direction = dip_switch_map[hardware_code]
            return InputEvent(
                event_type=EventType.DIP_SWITCH,
                dip_id=dip_id,
                direction=direction,
                state=normalized_state,
            )

        # Regular button events (1-32)
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state,
            )

        # Unknown event
        return InputEvent(event_type=EventType.UNKNOWN)
    
    def set_frame_background(self, path):
        return self._set_frame_background(path)
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
        return {
            "size": (80, 80),
            "format": "PNG",
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
        self.feature_option.hasDIPSwitch = True
        self.feature_option.dipSwitchCounts = self.DIP_SWITCH_COUNT
        self.feature_option.deviceType = device_type.dock_xl
        self.feature_option.supportBackgroundGif = True
        self.feature_option.supportConfig = True
        pass
