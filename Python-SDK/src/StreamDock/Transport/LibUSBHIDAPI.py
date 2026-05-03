"""
LibUSBHIDAPI - Pure-Python StreamDock Transport implementation

Drop-in replacement for the ctypes-based wrapper. Speaks the StreamDock HID
protocol directly using the ``hid`` package -- no compiled C library needed.
"""

import struct
import threading

from typing import Optional, Union, List
from ctypes import c_char_p
from os import PathLike

import hid


class _HidDeviceInfo:
    """Drop-in for the hid_device_info ctypes Structure used by the SDK's DeviceManager."""

    def __init__(self):
        self.path: str = ""
        self.vendor_id: int = 0
        self.product_id: int = 0
        self.serial_number: str = ""
        self.release_number: int = 0
        self.manufacturer_string: str = ""
        self.product_string: str = ""
        self.usage_page: int = 0
        self.usage: int = 0
        self.interface_number: int = 0
        self.next = None


class LibUSBHIDAPI:
    """Pure-Python transport for StreamDock devices.

    API-compatible with the ctypes-based original. Uses the ``hid`` package
    for raw HID I/O and constructs protocol packets in-process.
    """

    _DEFAULT_REPORT_SIZE = 1024

    def __init__(self, device_info: Optional[_HidDeviceInfo] = None):
        """
        Initialize the transport.

        Args:
            device_info: HID device information structure. If None, creates an uninitialized handle.
        """
        self._device_info = device_info
        self._device: Optional[hid.device] = None
        self._is_open = False
        self._report_id: int = 0
        self._input_report_size: int = 0
        self._output_report_size: int = 0
        self._feature_report_size: int = 0
        self._write_lock = threading.RLock()

    def __del__(self):
        self.close()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False

    # ------------------------------------------------------------------
    # Internal helpers
    # ------------------------------------------------------------------

    @property
    def _report_size(self) -> int:
        if self._output_report_size > 0:
            return self._output_report_size - 1
        else:
            return self._DEFAULT_REPORT_SIZE

    def _pad(self, buffer: bytes):
        return buffer + b"\x00" * (self._report_size - len(buffer))

    def _crt(self, cmd: str, params: bytes = b"", bulk: bytes = b"", crt: bytes = b"CRT\x00\x00"):
        """
        Send a `CRT` command to the device.

        Args:
            cmd: Command to execute
            params: Additional parameters that follow the command. Optional byte string, usually `struct.pack()`-ed data.
            bulk: Bulk data that will be streamed to the device after the command has been sent. Usually image data.
            crt: Allows to override the `CRT` command header. Useful for non-standard commands.
        """
        with self._write_lock:
            if self._device is None:
                return

            pkt = crt + cmd.encode("ascii") + params
            # print(f"write {pkt}")
            pkt = self._pad(pkt)
            self._device.write(self._pad(pkt))

            if len(bulk) > 0:
                for i in range(0, len(bulk), self._report_size):
                    # print(f"bulk {i=} {bulk[i:i+16]=} ")
                    stuff = b""
                    if bulk[i] == 0:
                        # for some reason hidapi seems to drop a leading nullbyte,
                        # we duplicate it, so the device receives the correct message
                        stuff = b"\x00"
                    self._device.write(self._pad(stuff + bulk[i:i+self._report_size]))

    # ------------------------------------------------------------------
    # Device lifecycle
    # ------------------------------------------------------------------

    def open(self, device_path: bytes) -> bool:
        """
        Open a device connection using the device path.

        Args:
            device_path: Device path as bytes
        """

        if self._is_open or self._device is not None:
            print("[WARNING] Device already open", flush=True)
            return False

        try:
            self._device = hid.device()
            assert self._device is not None
            self._device.open_path(device_path)
            self._device.set_nonblocking(False)
            self._is_open = True
            return True
        except Exception as e:
            print(f"[ERROR] Failed to open device: {e}", flush=True)
            return False

    def close(self):
        """
        Close the device connection.
        """
        try:
            if self._device is not None:
                self._device.close()
        except Exception as e:
            print(f"[WARNING] Error closing device: {e}", flush=True)
        finally:
            self._device = None
            self._is_open = False

    # ------------------------------------------------------------------
    # Device info / status
    # ------------------------------------------------------------------

    def get_firmware_version(self) -> str:
        if not self._device:
            return ""

        # TODO: does this work?
        try:
            buf = bytes([self._report_id]) + b"\x00" * self._report_size
            result = self._device.get_input_report(buf)
            # Response starts after report ID byte
            raw = bytes(result[1:])
            # Strip trailing nulls
            firmware = raw.split(b"\x00")[0]
            return firmware.decode("utf-8", errors="ignore") if firmware else ""
        except Exception:
            return ""

    def clear_task_queue(self):
        pass  # No-op in pure Python (no internal task queue)

    def can_write(self) -> bool:
        return self._device is not None and self._is_open

    # ------------------------------------------------------------------
    # Read
    # ------------------------------------------------------------------

    def read(self, timeout_ms: int = -1) -> Optional[bytes]:
        if not self._device:
            return None
        try:
            size = max(self._input_report_size, 1024) if self._input_report_size else 1024
            data = self._device.read(size, timeout_ms=timeout_ms)
            # if len(data) > 0:
            #     _data = bytes(data).rstrip(b"\x00")
            #     print(f"read() {_data=}")
            return bytes(data) if data else None
        except Exception:
            return None

    def read_(self, size: int) -> Optional[bytes]:
        """Read with fixed buffer size and 100ms timeout (matches original behavior)."""
        # TODO: actually honor size
        return self.read(timeout_ms=100)

    # ------------------------------------------------------------------
    # Screen control
    # ------------------------------------------------------------------

    def wakeup_screen(self):
        """Wake up the device screen."""
        self._crt("DIS")

    def magnetic_calibration(self):
        """Perform magnetic calibration."""
        self._crt("CHECK")

    def refresh_screen(self):
        """Refresh the screen display."""
        self._crt("STP")

    def sleep(self):
        """Put the device into sleep mode."""
        self._crt("HAN")

    # ------------------------------------------------------------------
    # Key control
    # ------------------------------------------------------------------

    def set_key_brightness(self, brightness: int):
        """
        Set the brightness of keys.

        Args:
            brightness: Brightness value, typically 0-100
        """
        self._crt("LIG", struct.pack(">HB", 0, brightness))

    def clear_all_keys(self):
        """Clear all keys on the device."""
        self.clear_key(0xff)

    def clear_key(self, key_index: int):
        """
        Clear the content of a specific key.

        Args:
            key_index: Index of the key to clear
        """
        self._crt("CLE", struct.pack(">HB", 0, key_index))

    # ------------------------------------------------------------------
    # Image transfer
    # ------------------------------------------------------------------

    def set_key_image_stream(self, jpeg_data: bytes, key_index: int):
        """
        Set a JPEG image to a specific key.

        Args:
            jpeg_data: JPEG image data
            key_index: Target key index
        """
        self._crt("BAT", struct.pack(">IB", len(jpeg_data), key_index), jpeg_data)

    def set_background_bitmap(self, bitmap_data: bytes, timeout_ms: int = 5000):
        """
        Set the full-screen background using raw bitmap data.

        Args:
            bitmap_data: Raw bitmap bytes
            timeout_ms: Transmission timeout in milliseconds
        """
        self.set_key_image_stream(bitmap_data, 1)

    def set_background_image_stream(self, jpeg_data: bytes, timeout_ms: int = 3000):
        """
        Set a JPEG image as full-screen background.

        Args:
            jpeg_data: JPEG image data
            timeout_ms: Transmission timeout in milliseconds
        """
        self.set_key_image_stream(jpeg_data, 1)

    def set_background_frame_stream(
        self,
        jpeg_data: bytes,
        width: int,
        height: int,
        x: int = 0,
        y: int = 0,
        fb_layer: int = 0x00,
    ) -> None:
        """
        Draw a JPEG frame at a specific position (used for animated backgrounds).

        Args:
            jpeg_data: JPEG image data
            width: Image width
            height: Image height
            x: X-coordinate position
            y: Y-coordinate position
            fb_layer: Framebuffer layer index
        """
        self._crt("BGPIC", struct.pack(">I4H2B", len(jpeg_data), x, y, width, height, 0, fb_layer), jpeg_data)

    def clear_background_frame_stream(self, position: int = 0x03):
        """
        Clear background frame on the specified framebuffer layer.

        Args:
            position: Layer index (default 0x03)
        """
        self._crt("BGCLE", struct.pack(">HB", 0, position))

    # ------------------------------------------------------------------
    # LED control
    # ------------------------------------------------------------------

    def set_led_brightness(self, brightness: int):
        """
        Set LED brightness.

        Args:
            brightness: Brightness value, typically 0-100
        """
        self._crt("LBLIG", struct.pack(">B", brightness))

    def set_led_color(self, count: int, r: int, g: int, b: int):
        """
        Set color for the first N LEDs.

        Args:
            count: Number of LEDs to set
            r: Red component (0-255)
            g: Green component (0-255)
            b: Blue component (0-255)
        """
        assert 0 <= count <= 4
        rgb = ((r, g, b) * count) + ((0, 0, 0) * (4 - count))
        self._crt("SETLB", struct.pack(">12B", *rgb))

    def reset_led_color(self):
        """Reset LED colors to default."""
        self._crt("DELED")

    def set_led_color_individual(self, a: tuple[int, int, int], b: tuple[int, int, int], c: tuple[int, int, int], d: tuple[int, int, int]):
        """Set color of the 4 LEDs individually."""
        rgb = a + b + c + d
        self._crt("SETLB", struct.pack(">12B", *rgb))

    # ------------------------------------------------------------------
    # Keyboard control
    # ------------------------------------------------------------------

    def set_keyboard_backlight_brightness(self, brightness: int):
        """
        Set the keyboard backlight brightness.

        Args:
            brightness: Brightness value (0-6)
        """
        assert 0 <= brightness <= 6
        self._crt("LLUM", struct.pack(">H", brightness))

    def set_keyboard_lighting_effects(self, effect: int):
        """
        Set the keyboard lighting effect.
        0 is static lighting.
        Args:
            effect: Effect mode identifier (0-9)
        """
        assert 0 <= effect <= 9
        self._crt("LMOD", struct.pack(">H", effect))

    def set_keyboard_lighting_speed(self, speed: int):
        """
        Set the keyboard lighting effect speed.

        Args:
            speed: Speed value for lighting effects (0-7)
        """
        assert 0 <= speed <= 7
        self._crt("LSPE", struct.pack(">H", speed))

    def set_keyboard_rgb_backlight(self, red: int, green: int, blue: int):
        """
        Set the keyboard RGB backlight color.

        Args:
            red: Red component (0-255)
            green: Green component (0-255)
            blue: Blue component (0-255)
        """
        self._crt("COLOR", struct.pack(">6B", 0, 0, 0, red, green, blue))

    def keyboard_os_mode_switch(self, os_mode: int) -> None:
        """
        Switch the keyboard OS mode.

        Args:
            os_mode: OS mode enum value (e.g., 0 for Windows, 1 for macOS)
        """
        assert 0 <= os_mode <= 1
        m = ord("M") if os_mode == 1 else ord("W")
        self._crt("CPOS", struct.pack(">H", m))

    # ------------------------------------------------------------------
    # Device configuration
    # ------------------------------------------------------------------

    def set_device_config(self, configs: List[int]):
        """
        Send raw configuration data to the device.

        Args:
            configs: List of configuration byte values
        """
        self._crt("QUCMD", bytes(configs))

    def change_mode(self, mode: int):
        """
        Change device working mode.

        Args:
            mode: Mode identifier
        """
        self._crt("MOD", struct.pack(">B", ord("1") + mode))

    def change_page(self, page: int):
        """
        Change N1 device calculator mode working page.

        Args:
            page: Page identifier
        """
        self._crt("M_V", struct.pack(">B", page), crt=b"")

    def set_n1_skin_bitmap(
        self,
        jpeg_data: bytes,
        skin_mode: int,
        skin_page: int,
        skin_status: int,
        key_index: int,
        timeout_ms: int = 3000,
    ):
        """
        Set N1 skin bitmap for a specific mode, page, and key.
        Args:
            jpeg_data: JPEG image data for the skin
            skin_mode: Skin mode identifier, 0 for keyboard, 1 for keyboard lock, 2 for calculator
            skin_page: Skin page identifier, 1-5
            skin_status: Skin status identifier, 0 for press, 1 for release
            key_index: Target key index for the skin, calculator (1-15), keyboard (1-18)
            timeout_ms: Transmission timeout in milliseconds
        """
        self._crt("LOG", struct.pack(">I4B", len(jpeg_data), skin_mode, skin_page, skin_status, key_index), jpeg_data, crt=b"CRT\xff\x00")

    # ------------------------------------------------------------------
    # Heartbeat / disconnect
    # ------------------------------------------------------------------

    def heartbeat(self):
        """Send a heartbeat packet to the device."""
        self._crt("CONNECT")

    def notify_disconnected(self):
        """Notify the device of disconnection."""
        self._crt("CLE\x00\x00DC")

    # ------------------------------------------------------------------
    # Report configuration
    # ------------------------------------------------------------------

    def set_report_id(self, report_id: int):
        """
        Set the report ID used for communication.

        Args:
            report_id: Report ID value (default is typically 0x01)
        """
        self._report_id = report_id & 0xFF

    def get_report_id(self) -> int:
        """
        Get the current report ID.

        Returns:
            int: Current report ID value
        """
        return self._report_id

    def set_report_size(
        self, input_report_size: int, output_report_size: int, feature_report_size: int
    ):
        """
        Set the sizes of the input, output, and feature reports.

        Args:
            input_report_size: Input report length
            output_report_size: Output report length
            feature_report_size: Feature report length
        """
        self._input_report_size = input_report_size
        self._output_report_size = output_report_size
        self._feature_report_size = feature_report_size

    # ------------------------------------------------------------------
    # Error handling
    # ------------------------------------------------------------------

    def get_last_error(self) -> str:
        """
        Get the last raw HID error message.

        Returns:
            str: Error message string
        """
        if self._device is None:
            return ""
        try:
            return str(self._device.error())
        except Exception:
            return ""

    def get_last_error_info(self) -> dict:
        err = self.get_last_error()
        return {"error_message": err} if err else {}

    # ------------------------------------------------------------------
    # Static methods
    # ------------------------------------------------------------------

    @staticmethod
    def disable_output(disable: bool = True):
        pass  # Debug log control — no-op in pure Python

    @staticmethod
    def create_device_info_from_dict(device_dict: dict) -> _HidDeviceInfo:
        """
        Create a _HidDeviceInfo structure from a device dictionary.

        Args:
            device_dict: Device information dictionary

        Returns:
            _HidDeviceInfo structure
        """
        info = _HidDeviceInfo()
        info.path = device_dict.get("path", "")
        info.vendor_id = device_dict.get("vendor_id", 0)
        info.product_id = device_dict.get("product_id", 0)
        info.serial_number = device_dict.get("serial_number", "")
        info.release_number = device_dict.get("release_number", 0)
        info.manufacturer_string = device_dict.get("manufacturer_string", "")
        info.product_string = device_dict.get("product_string", "")
        info.usage_page = device_dict.get("usage_page", 0)
        info.usage = device_dict.get("usage", 0)
        info.interface_number = device_dict.get("interface_number", 0)
        info.next = None
        return info

    @staticmethod
    def enumerate_devices(vendor_id: int, product_id: int) -> List[dict]:
        """
        Enumerate HID devices matching the given vendor and product IDs.

        Use the C library's built-in hidapi for enumeration to avoid conflicts with Python's hidapi package.

        Args:
            vendor_id: USB vendor ID
            product_id: USB product ID

        Returns:
            List of device information dictionaries
        """
        device_list = []
        for info in hid.enumerate(vendor_id, product_id):
            if info.get("interface_number") != 0:
                continue
            device_list.append({
                "path": info.get("path", ""),
                "vendor_id": info.get("vendor_id", 0),
                "product_id": info.get("product_id", 0),
                "serial_number": info.get("serial_number", ""),
                "manufacturer_string": info.get("manufacturer_string", ""),
                "product_string": info.get("product_string", ""),
                "release_number": info.get("release_number", 0),
                "usage_page": info.get("usage_page", 0),
                "usage": info.get("usage", 0),
                "interface_number": info.get("interface_number", 0),
            })
        return device_list

    # ------------------------------------------------------------------
    # Properties
    # ------------------------------------------------------------------

    @property
    def input_report_size(self) -> int:
        """Get the input report size."""
        return self._input_report_size

    @property
    def output_report_size(self) -> int:
        """Get the output report size."""
        return self._output_report_size

    @property
    def feature_report_size(self) -> int:
        """Get the feature report size."""
        return self._feature_report_size

    # ------------------------------------------------------------------
    # Legacy method aliases (backward compatibility)
    # ------------------------------------------------------------------

    def getFirmwareVersion(self) -> str:
        """Legacy alias for get_firmware_version()."""
        return self.get_firmware_version()

    def clearTaskQueue(self):
        """Legacy alias for clear_task_queue()."""
        self.clear_task_queue()

    def wakeScreen(self):
        """Legacy alias for wakeup_screen()."""
        self.wakeup_screen()

    def keyClear(self, index: int):
        """Legacy alias for clear_key()."""
        self.clear_key(index)

    def keyAllClear(self):
        """Legacy alias for clear_all_keys()."""
        self.clear_all_keys()

    def changePage(self, page: int):
        """Legacy alias for change_page()."""
        self.change_page(page)

    def switchMode(self, mode: int):
        """Legacy alias for change_mode()."""
        self.change_mode(mode)

    @staticmethod
    def _read_jpeg(path: Union[str, bytes, c_char_p, PathLike]) -> bytes:
        if isinstance(path, c_char_p):
            if path.value is None:
                raise ValueError("Path cannot be None")
            path = path.value
        with open(path, "rb") as fh:
            return fh.read()

    def setN1SkinBitMap(
        self,
        path: Union[str, bytes, c_char_p, PathLike],
        skin_mode: int,
        skin_page: int,
        skin_status: int,
        key_index: int,
    ):
        """
        Legacy method to set N1 skin bitmap from an image file path.
        Args:
            path: Path to the image file (can be str, bytes, c_char_p, or os.PathLike)
            skin_mode: Skin mode identifier, 0 for keyboard, 1 for keyboard lock, 2 for calculator
            skin_page: Skin page identifier, 1-5
            skin_status: Skin status identifier, 0 for press, 1 for release
            key_index: Target key index for the skin, calculator (1-15), keyboard (1-18)
        """
        try:
            self.set_n1_skin_bitmap(
                self._read_jpeg(path), skin_mode, skin_page, skin_status, key_index
            )
        except Exception as e:
            raise RuntimeError(f"Failed to load image from {path}: {e}")

    def setBackgroundImg(self, buffer: bytes, size: int):
        """
        Legacy method: Set background image from buffer.

        Args:
            buffer: Image data buffer
            size: Size of the buffer
        """
        self.set_background_bitmap(buffer[:size])

    def setBackgroundImgDualDevice(self, path: Union[str, bytes, c_char_p, PathLike]):
        """
        Legacy method: Set background image from file path (for dual device).

        Args:
            path: Path to the image file (can be str, bytes, c_char_p, or os.PathLike)
        """
        try:
            self.set_background_image_stream(self._read_jpeg(path))
        except Exception as e:
            raise RuntimeError(f"Failed to load image from {path}: {e}")

    def setBackgroundImgFrame(self, path: Union[str, bytes, c_char_p, PathLike], img_width: int, img_height: int):
        """
        Legacy method: Set Temporary background image from file path (for dual device).

        Args:
            path: Path to the image file (can be str, bytes, c_char_p, or os.PathLike)
            img_width: Width of the image
            img_height: Height of the image
        """
        try:
            self.set_background_frame_stream(self._read_jpeg(path), img_width, img_height)
        except Exception as e:
            raise RuntimeError(f"Failed to load image from {path}: {e}")

    def setKeyImg(self, path: Union[str, bytes, c_char_p, PathLike], key: int):
        """
        Legacy method: Set key image from file path.

        Args:
            path: Path to the image file (can be str, bytes, c_char_p, or os.PathLike)
            key: Key index
        """
        try:
            return self.set_key_image_stream(self._read_jpeg(path), key)
        except Exception as e:
            raise RuntimeError(f"Failed to load image from {path}: {e}")

    def setKeyImgDualDevice(self, path, key: int):
        """
        Legacy method: Set key image from file path (for dual device).

        Args:
            path: Path to the image file (can be str, bytes, c_char_p, or os.PathLike)
            key: Key index
        """
        return self.setKeyImg(path, key)

    def setKeyImgDataDualDevice(self, data: bytes, key: int):
        """
        Legacy method: Set key image from data buffer (for dual device).

        Args:
            data: Image data as bytes
            key: Key index
        """
        self.set_key_image_stream(data, key)

    def setBrightness(self, percent: int):
        """
        Legacy method: Set brightness.

        Args:
            percent: Brightness percentage (0-100)
        """
        self.set_key_brightness(percent)

    def disconnected(self):
        """Legacy method: Notify device of disconnection."""
        self.notify_disconnected()

    def refresh(self):
        """Legacy method: Refresh the display."""
        self.refresh_screen()
