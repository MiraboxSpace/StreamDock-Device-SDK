from StreamDock.FeatrueOption import device_type
from .StreamDock import StreamDock
from ..DeviceConfig import StreamDockN4ProConfig
from ..InputTypes import InputEvent, ButtonKey, EventType, KnobId, Direction


class StreamDockN4Pro(StreamDock):
    """StreamDockN4Pro device class - supports 14 image slots, 4 knobs, and swipe gestures"""

    KEY_COUNT = 15
    KEY_MAP = False

    # Image key mapping: logical key -> hardware key (for setting images)
    _IMAGE_KEY_MAP = {
        ButtonKey.KEY_1: 1,
        ButtonKey.KEY_2: 2,
        ButtonKey.KEY_3: 3,
        ButtonKey.KEY_4: 4,
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
        0x40: ButtonKey.KEY_1,
        0x41: ButtonKey.KEY_2,
        0x42: ButtonKey.KEY_3,
        0x43: ButtonKey.KEY_4,
    }

    def __init__(self, transport1, devInfo):
        super().__init__(transport1, devInfo)
        self.config = StreamDockN4ProConfig()

    def decode_touch_bar_event(self, data):
        """
        Decode an N4Pro touch-bar raw packet into an InputEvent.

        The N4Pro touch packet header is ACK ARX and stores the touch point as
        big-endian X/Y coordinates at bytes 10-13.
        """
        if data is None or len(data) < 14:
            return None

        is_touch_packet = (
            data[0] == 0x41
            and data[1] == 0x43
            and data[2] == 0x4B
            and data[4] == 0x41
            and data[5] == 0x52
            and data[6] == 0x58
        )
        if not is_touch_packet:
            return None

        x_pos = (data[10] << 8) | data[11]
        y_pos = (data[12] << 8) | data[13]
        return InputEvent(
            event_type=EventType.TOUCH_POINT,
            x=x_pos,
            y=y_pos,
            raw_data=bytes(data),
        )

    def _handle_raw_read(self, data):
        super()._handle_raw_read(data)

        event = self.decode_touch_bar_event(data)
        if event is None:
            return

        with self._callback_lock:
            callback = self.touchscreen_callback
            async_run = self.touchscreen_callback_async

        if callback is None:
            return

        if async_run:
            import threading

            threading.Thread(target=callback, args=(self, event), daemon=True).start()
        else:
            callback(self, event)

    def set_touch_bar_callback(self, callback, async_run=False):
        """
        Register a callback for N4Pro touch-bar points.

        Callback signature:
            callback(device: StreamDockN4Pro, event: InputEvent)

        The touch coordinates are available as event.x and event.y.
        """
        self.set_touchscreen_callback(callback, async_run=async_run)

    def register_touch_bar_callback(self, callback, async_run=False):
        return self.set_touch_bar_callback(callback, async_run=async_run)

    def registerTouchBarCallback(self, callback, asyncRun=False):
        return self.set_touch_bar_callback(callback, async_run=asyncRun)

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
        raise ValueError(f"StreamDockN4Pro: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        Decode hardware event codes into a unified InputEvent

        Hardware code mapping:
        - Main input keys: 6-15; secondary input keys: 1-4
        - Secondary screen keys: 0x40-0x43
        - Knob rotation: 0xA0, 0xA1(Knob1), 0x50, 0x51(Knob2), 0x90, 0x91(Knob3), 0x70, 0x71(Knob4)
        - Knob press: 0x37(Knob1), 0x35(Knob2), 0x33(Knob3), 0x36(Knob4)
        - Swipe: 0x38 (left), 0x39 (right)
        """
        # State values: 0x00=release, 0x01=press
        if state not in (0, 1):
            return InputEvent(event_type=EventType.UNKNOWN)
        normalized_state = 1 if state == 0x01 else 0

        if hardware_code in range(0x40, 0x44) and state != 0:
            return InputEvent(event_type=EventType.UNKNOWN)

        # Main and secondary input buttons.
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=normalized_state,
            )


        # Knob rotation event
        knob_rotate_map = {
            0xA0: (KnobId.KNOB_1, Direction.LEFT),
            0xA1: (KnobId.KNOB_1, Direction.RIGHT),
            0x50: (KnobId.KNOB_2, Direction.LEFT),
            0x51: (KnobId.KNOB_2, Direction.RIGHT),
            0x90: (KnobId.KNOB_3, Direction.LEFT),
            0x91: (KnobId.KNOB_3, Direction.RIGHT),
            0x70: (KnobId.KNOB_4, Direction.LEFT),
            0x71: (KnobId.KNOB_4, Direction.RIGHT),
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
            0x37: KnobId.KNOB_1,
            0x35: KnobId.KNOB_2,
            0x33: KnobId.KNOB_3,
            0x36: KnobId.KNOB_4,
        }
        if hardware_code in knob_press_map:
            return InputEvent(
                event_type=EventType.KNOB_PRESS,
                knob_id=knob_press_map[hardware_code],
                state=normalized_state,
            )

        # Swipe gesture
        if hardware_code == 0x38 and state == 0:
            return InputEvent(event_type=EventType.SWIPE, direction=Direction.LEFT)
        if hardware_code == 0x39 and state == 0:
            return InputEvent(event_type=EventType.SWIPE, direction=Direction.RIGHT)

        # Unknown event
        return InputEvent(event_type=EventType.UNKNOWN)

    # Set device screen brightness
    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)

    # Set the boot logo using the model's input canvas.
    def set_touchscreen_image(self, path):
        return self._set_boot_logo(path)
    def set_frame_background(self, path):
        return self._set_frame_background(path)

    # Set an image on a valid display slot.
    def set_key_image(self, key, path):
        return self._set_key_image(key, path)

    # Set an image on secondary display slots 1-4.
    def set_seondscreen_image(self, key, path):
        if key not in range(1, 5):
            return -1
        return self._set_key_image(key, path)

    # TODO
    def set_key_imageData(self, key, path):
        pass

    # Get device serial number
    def get_serial_number(self):
        return self.serial_number

    def key_image_format(self, key=None):
        if key is not None and key in range(1, 5):
            return self.secondscreen_image_format()
        return {
            "size": (112, 112),
            "format": "PNG",
            "rotation": 180,
            "flip": (False, False),
        }

    def secondscreen_image_format(self):
        return {
            "size": (176, 112),
            "format": "PNG",
            "rotation": 180,
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
        self.feature_option.hasRGBLed = True
        self.feature_option.ledCounts = 4
        self.feature_option.deviceType = device_type.dock_n4pro
        self.feature_option.supportBackgroundGif = True
        self.feature_option.supportConfig = True
        self.feature_option.support_single_led_color = True
        pass
