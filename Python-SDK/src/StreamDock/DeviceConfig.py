from dataclasses import dataclass, fields


CONFIG_DEFAULT = 0x11
CONFIG_ON = 0x11
CONFIG_OFF = 0xFF


@dataclass
class DeviceConfig:
    """Small boolean config object that serializes to StreamDock config bytes."""

    def to_bytes(self):
        values = []
        for field in fields(self):
            value = getattr(self, field.name)
            if value is None:
                values.append(CONFIG_DEFAULT)
            elif value is True:
                values.append(CONFIG_ON)
            elif value is False:
                values.append(CONFIG_OFF)
            else:
                raise ValueError(f"{field.name} must be True, False, or None")
        return values

    def reset(self):
        for field in fields(self):
            setattr(self, field.name, None)


@dataclass
class StreamDockN4ProConfig(DeviceConfig):
    led_follow_key_light: bool | None = None
    key_light_on_disconnect: bool | None = None
    check_usb_power: bool | None = None
    enable_vibration: bool | None = None
    reset_usb_report: bool | None = None
    enable_boot_video: bool | None = None


@dataclass
class StreamDockXLConfig(DeviceConfig):
    led_follow_key_light: bool | None = None
