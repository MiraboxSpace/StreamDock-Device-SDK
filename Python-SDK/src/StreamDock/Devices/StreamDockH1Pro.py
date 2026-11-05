import os
import shutil
import subprocess
import tempfile
from enum import IntEnum

from PIL import Image

from .StreamDock import StreamDock
from ..FeatrueOption import device_type
from ..InputTypes import ButtonKey, EventType, InputEvent


class StreamDockH1Pro(StreamDock):
    """H1 Pro / H1 ProE: 12 keys in 3 rows and 4 columns, without knobs."""

    KEY_COUNT = 12
    KEY_ROWS = 3
    KEY_COLS = 4

    class DeviceMode(IntEnum):
        SCREENSAVER = 0
        KEY = 1
        GIF = 2

    # Hardware key IDs, from left to right and top to bottom:
    # 01 02 03 04
    # 05 06 07 08
    # 09 0A 0B 0C
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
    }
    _HW_TO_LOGICAL_KEY = {
        0x01: ButtonKey.KEY_1,
        0x02: ButtonKey.KEY_2,
        0x03: ButtonKey.KEY_3,
        0x04: ButtonKey.KEY_4,
        0x05: ButtonKey.KEY_5,
        0x06: ButtonKey.KEY_6,
        0x07: ButtonKey.KEY_7,
        0x08: ButtonKey.KEY_8,
        0x09: ButtonKey.KEY_9,
        0x0A: ButtonKey.KEY_10,
        0x0B: ButtonKey.KEY_11,
        0x0C: ButtonKey.KEY_12,
    }

    def get_image_key(self, logical_key: ButtonKey) -> int:
        if logical_key in self._IMAGE_KEY_MAP:
            return self._IMAGE_KEY_MAP[logical_key]
        raise ValueError(f"StreamDockH1Pro: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        if state not in (0, 1):
            return InputEvent(event_type=EventType.UNKNOWN)
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=1 if state == 0x01 else 0,
            )
        return InputEvent(event_type=EventType.UNKNOWN)

    def switch_mode(self, mode: DeviceMode | int):
        """Select screensaver (0), key (1), or animated image (2) mode."""
        return self.transport.switchMode(self.DeviceMode(mode).value)

    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)


    def set_key_image(self, key, path):
        return self._set_key_image(key, path)

    def set_touchscreen_image(self, path):
        return self._set_boot_logo(path)

    def set_frame_background(self, path):
        return self._set_frame_background(path)

    def upload_gif(self, path):
        """Upload a GIF file to device storage without changing the current mode."""
        with Image.open(path) as image:
            if image.format != "GIF":
                raise ValueError("Expected a GIF file")
        return self._upload_animation(path)

    def _upload_animation(self, path):
        ffmpeg = shutil.which("ffmpeg")
        if ffmpeg is None:
            raise RuntimeError("ffmpeg is required for H1 Pro video conversion")
        # The panel is portrait: rotate frames 90° counterclockwise like the
        # static touchscreen path (touchscreen_image_format rotation: 90) and
        # fit the result onto a 240x320 canvas.
        scale = (
            "transpose=2,"
            "scale=240:320:force_original_aspect_ratio=decrease,"
            "pad=240:320:(ow-iw)/2:(oh-ih)/2,format=yuvj420p"
        )
        attempts = [(30, 6), (20, 10), (15, 14), (10, 18),
                    (8, 20), (5, 25), (2, 31)]
        storage_error = None
        with tempfile.TemporaryDirectory(prefix="streamdock_h1pro_video_") as directory:
            output = os.path.join(directory, "animation.mp4")
            for rate, quality in attempts:
                result = subprocess.run(
                    [ffmpeg, "-hide_banner", "-loglevel", "error", "-y",
                     "-i", os.fspath(path), "-an", "-vf", f"fps={rate},{scale}",
                     "-c:v", "mjpeg", "-q:v", str(quality), output],
                    capture_output=True, text=True,
                )
                if result.returncode != 0:
                    raise RuntimeError(f"Video conversion failed: {result.stderr.strip()}")
                if 0 < os.path.getsize(output) <= 5 * 1024 * 1024:
                    with open(output, "rb") as video:
                        try:
                            return self.transport.upload_h1pro_video(video.read())
                        except RuntimeError as exc:
                            if "H1 Pro video exceeds available device storage" not in str(exc):
                                raise
                            storage_error = exc
        if storage_error is not None:
            raise RuntimeError("Video cannot fit in H1 Pro device storage") from storage_error
        raise ValueError("Video exceeds 5 MiB after H1 Pro conversion")

    def set_background_gif(self, path, x=0, y=0, fb_layer=0x00):
        raise NotImplementedError("H1 Pro has no background GIF; use upload_gif() and switch_mode(DeviceMode.GIF)")

    def set_background_mp4(self, path, x=0, y=0, fb_layer=0x00, fps=None):
        raise NotImplementedError("H1 Pro has no background video; use upload_gif() and switch_mode(DeviceMode.GIF)")

    def set_background_gif_stream(self, frames, delays, x=0, y=0, fb_layer=0x00):
        raise NotImplementedError("H1 Pro supports file upload, not host-side JPEG frame streaming")

    def get_serial_number(self):
        return self.serial_number

    def key_image_format(self, key=None):
        new_layout = any(tag in self.firmware_version.upper() for tag in ("V25", "V3"))
        return {
            "size": (64, 64) if new_layout else (70, 70), "format": "JPEG",
            "rotation": 90 if new_layout else 0, "flip": (False, False),
        }

    def touchscreen_image_format(self):
        new_layout = any(tag in self.firmware_version.upper() for tag in ("V25", "V3"))
        return {
            "size": (320, 240), "format": "JPEG" if new_layout else "RAW",
            "pixel_format": "RGB565",
            "rotation": 90 if new_layout else -90, "flip": (False, False),
        }

    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.deviceType = device_type.dock_h1pro
        self.feature_option.supportBackgroundGif = False
