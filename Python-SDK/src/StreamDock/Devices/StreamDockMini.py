from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..InputTypes import InputEvent, ButtonKey, EventType, DIPSwitchId, Direction


class StreamDockMini(StreamDock):
    """StreamDockMini device class - supports 6 keys and 2 DIP switches"""

    KEY_COUNT = 6
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
    }

    # Input codes are separate from image-slot numbers.
    _HW_TO_LOGICAL_KEY = {
        0x01: ButtonKey.KEY_1,
        0x02: ButtonKey.KEY_2,
        0x03: ButtonKey.KEY_3,
        0x04: ButtonKey.KEY_4,
        0x05: ButtonKey.KEY_5,
        0x06: ButtonKey.KEY_6,
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
        if state not in (0, 1):
            return InputEvent(event_type=EventType.UNKNOWN)
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
        self.feature_option.hasDIPSwitch = True
        self.feature_option.dipSwitchCounts = self.DIP_SWITCH_COUNT
        self.feature_option.deviceType = device_type.dock_mini
        pass
