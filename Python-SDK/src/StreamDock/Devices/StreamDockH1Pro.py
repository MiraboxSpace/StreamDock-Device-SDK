import ctypes
import os
import shutil
import subprocess
import tempfile
from enum import IntEnum

from PIL import Image

from .StreamDock import StreamDock
from ..FeatrueOption import device_type
from ..ImageHelpers.PILHelper import to_native_key_format, to_native_touchscreen_format
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
    _IMAGE_KEY_MAP = {ButtonKey(key): key for key in range(1, 13)}
    _HW_TO_LOGICAL_KEY = {v: k for k, v in _IMAGE_KEY_MAP.items()}

    def get_image_key(self, logical_key: ButtonKey) -> int:
        if logical_key in self._IMAGE_KEY_MAP:
            return self._IMAGE_KEY_MAP[logical_key]
        raise ValueError(f"StreamDockH1Pro: Unsupported key {logical_key}")

    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        if hardware_code in self._HW_TO_LOGICAL_KEY:
            return InputEvent(
                event_type=EventType.BUTTON,
                key=self._HW_TO_LOGICAL_KEY[hardware_code],
                state=1 if state == 0x01 else 0,
            )
        return InputEvent(event_type=EventType.UNKNOWN)

    def switch_mode(self, mode: DeviceMode | int):
        """Select screensaver (1), key (2), or animated image (3) mode."""
        return self.transport.switchMode(self.DeviceMode(mode).value)

    def set_brightness(self, percent):
        return self.transport.setBrightness(percent)

    def _send_image(self, path, formatter, sender, *args):
        try:
            with Image.open(path) as source:
                image = formatter(self, source)
            with tempfile.TemporaryDirectory(prefix="streamdock_h1pro_") as directory:
                image_path = os.path.join(directory, "image.jpg")
                with image:
                    image.save(image_path, "JPEG", quality=95)
                return sender(ctypes.c_char_p(image_path.encode("utf-8")), *args)
        except Exception as e:
            print(f"Error: {e}")
            return -1

    def set_key_image(self, key, path):
        try:
            hardware_key = self.get_image_key(key)
        except (ValueError, TypeError) as e:
            print(f"Error: {e}")
            return -1
        return self._send_image(
            path, to_native_key_format, self.transport.setKeyImgDualDevice, hardware_key
        )

    def set_touchscreen_image(self, path):
        return self._send_image(
            path, to_native_touchscreen_format, self.transport.setBackgroundImgDualDevice
        )

    def set_frame_background(self, path):
        """Display a static image using the device's upload path."""
        return self.set_touchscreen_image(path)

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
        return {
            "size": (64, 64), "format": "JPEG",
            "rotation": 90, "flip": (False, False),
        }

    def touchscreen_image_format(self):
        return {
            "size": (320, 240), "format": "JPEG",
            "rotation": 90, "flip": (False, False),
        }

    def set_device(self):
        self.transport.set_report_size(513, 1025, 0)
        self.feature_option.deviceType = device_type.dock_h1pro
        self.feature_option.supportBackgroundGif = False
