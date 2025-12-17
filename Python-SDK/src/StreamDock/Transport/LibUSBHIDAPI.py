"""
LibUSBHIDAPI - Python wrapper for StreamDock Transport C library

This module provides a Python interface to the StreamDock Transport library,
encapsulating HID device operations such as:
- Device initialization and management
- I/O operations (read/write)
- Image transfer (key and background images)
- LED control
- Device configuration and control

The wrapper follows RAII-style resource management patterns and provides
a clean, Pythonic interface to the underlying C API.
"""

import os
import ctypes
import platform
from ctypes import (
    POINTER,
    c_size_t,
    c_uint8,
    c_void_p,
    c_char_p,
    c_int,
    c_ulong,
    c_ubyte,
    c_uint16,
    c_uint32,
    c_int32,
    c_wchar_p,
    c_char,
)
import re
from typing import Optional, List, Tuple


def _get_dll_name() -> str:
    """
    Determine the appropriate transport library name based on platform and architecture.

    Returns:
        str: The library filename to load

    Raises:
        RuntimeError: If the platform/architecture combination is not supported
    """
    search_library_names = {
        "Windows": {"x86_64": "transport.dll"},
        "Linux": {"x86_64": "libtransport.so", "aarch64": "libtransport_arm64.so"},
        "Darwin": {"x86_64": "libtransport.dylib", "arm64": "libtransport_arm64.dylib"},
    }

    platform_name = platform.system()
    machine_type = platform.machine().lower()

    if platform_name == "Windows":
        return search_library_names["Windows"]["x86_64"]
    elif platform_name == "Linux":
        if "x86_64" in machine_type or "amd64" in machine_type:
            return search_library_names["Linux"]["x86_64"]
        elif "aarch64" in machine_type or "arm64" in machine_type:
            return search_library_names["Linux"]["aarch64"]
    elif platform_name == "Darwin":
        if "x86_64" in machine_type or "amd64" in machine_type:
            return search_library_names["Darwin"]["x86_64"]
        elif "arm64" in machine_type:
            return search_library_names["Darwin"]["arm64"]

    raise RuntimeError(
        f"Unsupported platform/architecture: {platform_name} / {machine_type}"
    )


# Load the transport library
_dll_name = _get_dll_name()
_dll_path = os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "TransportDLL", _dll_name
)
_transport_lib = ctypes.CDLL(_dll_path)


class _HidDeviceInfo(ctypes.Structure):
    """
    Structure definition for the hid_device_info structure defined
    in the HIDAPI library.
    """

    pass


_HidDeviceInfo._fields_ = [
    ("path", ctypes.c_char_p),
    ("vendor_id", ctypes.c_ushort),
    ("product_id", ctypes.c_ushort),
    ("serial_number", ctypes.c_wchar_p),
    ("release_number", ctypes.c_ushort),
    ("manufacturer_string", ctypes.c_wchar_p),
    ("product_string", ctypes.c_wchar_p),
    ("usage_page", ctypes.c_ushort),
    ("usage", ctypes.c_ushort),
    ("interface_number", ctypes.c_int),
    ("next", ctypes.POINTER(_HidDeviceInfo)),
]


# Define C function signatures
_transport_lib.transport_create.restype = c_uint32  # TransportResult
_transport_lib.transport_create.argtypes = [POINTER(_HidDeviceInfo), POINTER(c_void_p)]

_transport_lib.transport_destroy.restype = c_uint32  # TransportResult
_transport_lib.transport_destroy.argtypes = [c_void_p]

_transport_lib.transport_get_firmware_version.restype = c_uint32
_transport_lib.transport_get_firmware_version.argtypes = [c_void_p, c_char_p, c_size_t]

_transport_lib.transport_clear_task_queue.restype = c_uint32
_transport_lib.transport_clear_task_queue.argtypes = [c_void_p]

_transport_lib.transport_can_write.restype = c_uint32
_transport_lib.transport_can_write.argtypes = [c_void_p, POINTER(c_int)]

_transport_lib.transport_read.restype = c_uint32  # TransportResult
_transport_lib.transport_read.argtypes = [
    c_void_p,
    POINTER(c_uint8),
    POINTER(c_size_t),
    c_int32,
]

_transport_lib.transport_wakeup_screen.restype = c_uint32
_transport_lib.transport_wakeup_screen.argtypes = [c_void_p]

_transport_lib.transport_set_key_brightness.restype = c_uint32
_transport_lib.transport_set_key_brightness.argtypes = [c_void_p, c_uint8]

_transport_lib.transport_clear_all_keys.restype = c_uint32
_transport_lib.transport_clear_all_keys.argtypes = [c_void_p]

_transport_lib.transport_clear_key.restype = c_uint32
_transport_lib.transport_clear_key.argtypes = [c_void_p, c_uint8]

_transport_lib.transport_refresh.restype = c_uint32
_transport_lib.transport_refresh.argtypes = [c_void_p]

_transport_lib.transport_sleep.restype = c_uint32
_transport_lib.transport_sleep.argtypes = [c_void_p]

_transport_lib.transport_disconnected.restype = c_uint32
_transport_lib.transport_disconnected.argtypes = [c_void_p]

_transport_lib.transport_heartbeat.restype = c_uint32
_transport_lib.transport_heartbeat.argtypes = [c_void_p]

_transport_lib.transport_set_background_bitmap.restype = c_uint32
_transport_lib.transport_set_background_bitmap.argtypes = [
    c_void_p,
    c_char_p,
    c_size_t,
    c_uint32,
]

_transport_lib.transport_set_key_image_stream.restype = c_uint32
_transport_lib.transport_set_key_image_stream.argtypes = [
    c_void_p,
    c_char_p,
    c_size_t,
    c_uint8,
]

_transport_lib.transport_set_background_image_stream.restype = c_uint32
_transport_lib.transport_set_background_image_stream.argtypes = [
    c_void_p,
    c_char_p,
    c_size_t,
    c_uint32,
]

_transport_lib.transport_set_background_frame_stream.restype = c_uint32
_transport_lib.transport_set_background_frame_stream.argtypes = [
    c_void_p,
    c_char_p,
    c_size_t,
    c_uint16,
    c_uint16,
    c_uint16,
    c_uint16,
    c_uint8,
]

_transport_lib.transport_clear_background_frame_stream.restype = c_uint32
_transport_lib.transport_clear_background_frame_stream.argtypes = [c_void_p, c_uint8]

_transport_lib.transport_set_led_brightness.restype = c_uint32
_transport_lib.transport_set_led_brightness.argtypes = [c_void_p, c_uint8]

_transport_lib.transport_set_led_color.restype = c_uint32
_transport_lib.transport_set_led_color.argtypes = [
    c_void_p,
    c_uint16,
    c_uint8,
    c_uint8,
    c_uint8,
]

_transport_lib.transport_reset_led_color.restype = c_uint32
_transport_lib.transport_reset_led_color.argtypes = [c_void_p]

_transport_lib.transport_set_device_config.restype = c_uint32
_transport_lib.transport_set_device_config.argtypes = [
    c_void_p,
    POINTER(c_uint8),
    c_size_t,
]

_transport_lib.transport_change_mode.restype = c_uint32
_transport_lib.transport_change_mode.argtypes = [c_void_p, c_uint8]

_transport_lib.transport_set_reportID.restype = c_uint32
_transport_lib.transport_set_reportID.argtypes = [c_void_p, c_uint8]

_transport_lib.transport_reportID.restype = c_uint32
_transport_lib.transport_reportID.argtypes = [c_void_p, POINTER(c_uint8)]

_transport_lib.transport_set_reportSize.restype = c_uint32
_transport_lib.transport_set_reportSize.argtypes = [
    c_void_p,
    c_uint16,
    c_uint16,
    c_uint16,
]

_transport_lib.transport_raw_hid_last_error.restype = c_uint32
_transport_lib.transport_raw_hid_last_error.argtypes = [
    c_void_p,
    ctypes.c_void_p,
    POINTER(c_size_t),
]

_transport_lib.transport_disable_output.restype = c_uint32
_transport_lib.transport_disable_output.argtypes = [ctypes.c_int8]

# Add missing function signature
_transport_lib.transport_get_last_error_info.restype = c_uint32  # TransportResult
_transport_lib.transport_get_last_error_info.argtypes = [
    c_void_p,
    c_void_p,
]  # TransportErrorInfo*

# Load hidapi functions directly from the transport library
# This prevents conflicts with Python's hidapi package
try:
    _transport_lib.transport_hid_enumerate.restype = POINTER(_HidDeviceInfo)
    _transport_lib.transport_hid_enumerate.argtypes = [c_uint16, c_uint16]

    _transport_lib.transport_hid_free_enumeration.restype = None
    _transport_lib.transport_hid_free_enumeration.argtypes = [POINTER(_HidDeviceInfo)]

    _HID_API_AVAILABLE = True
except AttributeError:
    _HID_API_AVAILABLE = False
    print("Warning: hidapi functions not available in transport library")


class LibUSBHIDAPI:
    """
    Python wrapper for the StreamDock Transport C library.

    This class provides a high-level, Pythonic interface to the underlying C transport library,
    managing HID device operations with RAII-style resource management.

    Features:
    - Device initialization and cleanup
    - I/O operations (read/write)
    - Image transfer (key images, background images, bitmap streams)
    - LED control (brightness, color)
    - Device configuration and mode switching
    - Firmware version retrieval

    Example:
        device_info = get_device_info()  # from hidapi
        device = LibUSBHIDAPI(device_info)
        device.set_led_color(1, 255, 0, 0)  # Set first LED to red
        device.set_key_brightness(50)
        firmware = device.get_firmware_version()
    """

    def __init__(self, device_info: Optional[_HidDeviceInfo] = None):
        """
        Initialize the transport wrapper.

        Args:
            device_info: HID device information structure. If None, creates an uninitialized handle.
        """
        self._handle = None
        self._input_report_size = 0
        self._output_report_size = 0
        self._feature_report_size = 0
        # CRITICAL: Store device_info properly for resource management
        self._device_info = device_info
        self._is_open = False

        # Don't create handle immediately, wait for open() call
        # This maintains compatibility with the existing StreamDock API

    def __del__(self):
        """Destructor - automatically releases the transport handle."""
        if self._handle:
            _transport_lib.transport_destroy(self._handle)
            self._handle = None

    def __enter__(self):
        """Context manager support."""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager cleanup."""
        self.__del__()
        return False

    # ========== Device Information and Status ==========

    def get_firmware_version(self) -> str:
        """
        Get the firmware version string from the device.

        Returns:
            str: Firmware version string
        """
        if not self._handle:
            return ""

        buffer_size = 64
        buffer = ctypes.create_string_buffer(buffer_size)
        result = _transport_lib.transport_get_firmware_version(
            self._handle, buffer, buffer_size
        )
        if result != 0:
            return ""
        raw = buffer.raw

        parts = raw.split(b"\x00")
        decoded = ""
        for part in parts:
            if part:
                try:
                    decoded = part.decode("utf-8", errors="ignore")
                    break
                except Exception:
                    # fallback to continue searching
                    continue

        return decoded

    def clear_task_queue(self) -> None:
        """Clear all pending data in the transport library's task queue."""
        if not self._handle:
            return
        _transport_lib.transport_clear_task_queue(self._handle)

    def can_write(self) -> bool:
        """
        Check if the device is currently writable.

        Returns:
            bool: True if device can accept write operations
        """
        if not self._handle:
            return False
        can_write_val = c_int()
        result = _transport_lib.transport_can_write(
            self._handle, ctypes.byref(can_write_val)
        )
        if result != 0:
            return False
        return bool(can_write_val.value)

    def read(self, timeout_ms: int = -1) -> Optional[bytes]:
        """
        Read data from the device.

        Args:
            timeout_ms: Timeout in milliseconds. -1 means blocking read.

        Returns:
            bytes: Data read from device, or None if error occurred
        """
        if not self._handle:
            return None

        buffer_size = max(self._input_report_size, 1024)
        response = (c_uint8 * buffer_size)()
        length = c_size_t(buffer_size)

        result = _transport_lib.transport_read(
            self._handle, response, ctypes.byref(length), timeout_ms
        )

        if result == 0:  # TRANSPORT_SUCCESS is 0
            return bytes(response[: length.value])
        return None

    # ========== Screen Control ==========

    def wakeup_screen(self) -> None:
        """Wake up the device screen."""
        if not self._handle:
            return
        _transport_lib.transport_wakeup_screen(self._handle)

    def refresh_screen(self) -> None:
        """Refresh the screen display."""
        if not self._handle:
            return
        _transport_lib.transport_refresh(self._handle)

    def sleep(self) -> None:
        """Put the device into sleep mode."""
        if not self._handle:
            return
        _transport_lib.transport_sleep(self._handle)

    # ========== Key Control ==========

    def set_key_brightness(self, brightness: int) -> None:
        """
        Set the brightness of keys.

        Args:
            brightness: Brightness value, typically 0-100
        """
        if not self._handle:
            return
        _transport_lib.transport_set_key_brightness(self._handle, brightness)

    def clear_all_keys(self) -> None:
        """Clear all keys on the device."""
        if not self._handle:
            return
        _transport_lib.transport_clear_all_keys(self._handle)

    def clear_key(self, key_index: int) -> None:
        """
        Clear the content of a specific key.

        Args:
            key_index: Index of the key to clear
        """
        if not self._handle:
            return
        _transport_lib.transport_clear_key(self._handle, key_index)

    # ========== Image Transfer ==========

    def set_background_bitmap(self, bitmap_data: bytes, timeout_ms: int = 5000) -> None:
        """
        Set the full-screen background using raw bitmap data.

        Args:
            bitmap_data: Raw bitmap bytes
            timeout_ms: Transmission timeout in milliseconds
        """
        if not self._handle:
            return
        _transport_lib.transport_set_background_bitmap(
            self._handle, bitmap_data, len(bitmap_data), timeout_ms
        )

    def set_key_image_stream(self, jpeg_data: bytes, key_index: int) -> None:
        """
        Set a JPEG image to a specific key.

        Args:
            jpeg_data: JPEG image data
            key_index: Target key index
        """
        if not self._handle:
            return
        res = _transport_lib.transport_set_key_image_stream(
            self._handle, jpeg_data, len(jpeg_data), key_index
        )
        return res

    def set_background_image_stream(
        self, jpeg_data: bytes, timeout_ms: int = 3000
    ) -> None:
        """
        Set a JPEG image as full-screen background.

        Args:
            jpeg_data: JPEG image data
            timeout_ms: Transmission timeout in milliseconds
        """
        if not self._handle:
            return
        _transport_lib.transport_set_background_image_stream(
            self._handle, jpeg_data, len(jpeg_data), timeout_ms
        )

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
        if not self._handle:
            return
        _transport_lib.transport_set_background_frame_stream(
            self._handle, jpeg_data, len(jpeg_data), width, height, x, y, fb_layer
        )

    def clear_background_frame_stream(self, position: int = 0x03) -> None:
        """
        Clear background frame on the specified framebuffer layer.

        Args:
            position: Layer index (default 0x03)
        """
        if not self._handle:
            return
        _transport_lib.transport_clear_background_frame_stream(self._handle, position)

    # ========== LED Control ==========

    def set_led_brightness(self, brightness: int) -> None:
        """
        Set LED brightness.

        Args:
            brightness: Brightness value, typically 0-100
        """
        if not self._handle:
            return
        _transport_lib.transport_set_led_brightness(self._handle, brightness)

    def set_led_color(self, count: int, r: int, g: int, b: int) -> None:
        """
        Set color for the first N LEDs.

        Args:
            count: Number of LEDs to set
            r: Red component (0-255)
            g: Green component (0-255)
            b: Blue component (0-255)
        """
        if not self._handle:
            return
        _transport_lib.transport_set_led_color(self._handle, count, r, g, b)

    def reset_led_color(self) -> None:
        """Reset LED colors to default."""
        if not self._handle:
            return
        _transport_lib.transport_reset_led_color(self._handle)

    # ========== Device Configuration ==========

    def set_device_config(self, configs: List[int]) -> None:
        """
        Send raw configuration data to the device.

        Args:
            configs: List of configuration byte values
        """
        if not self._handle:
            return
        config_array = (c_uint8 * len(configs))(*configs)
        _transport_lib.transport_set_device_config(
            self._handle, config_array, len(configs)
        )

    def change_mode(self, mode: int) -> None:
        """
        Change device working mode.

        Args:
            mode: Mode identifier
        """
        if not self._handle:
            return
        _transport_lib.transport_change_mode(self._handle, mode)

    def notify_disconnected(self) -> None:
        """Notify the device of disconnection."""
        if not self._handle:
            return
        _transport_lib.transport_disconnected(self._handle)

    def heartbeat(self) -> None:
        """Send a heartbeat packet to the device."""
        if not self._handle:
            return
        _transport_lib.transport_heartbeat(self._handle)

    # ========== Report Configuration ==========

    def set_report_id(self, report_id: int) -> None:
        """
        Set the report ID used for communication.

        Args:
            report_id: Report ID value (default is typically 0x01)
        """
        if not self._handle:
            return
        _transport_lib.transport_set_reportID(self._handle, report_id)

    def get_report_id(self) -> int:
        """
        Get the current report ID.

        Returns:
            int: Current report ID value
        """
        if not self._handle:
            return 0x00
        out_id = c_uint8()
        result = _transport_lib.transport_reportID(self._handle, ctypes.byref(out_id))
        if result != 0:
            return 0x00
        return int(out_id.value)

    def set_report_size(
        self, input_report_size: int, output_report_size: int, feature_report_size: int
    ) -> None:
        """
        Set the sizes of the input, output, and feature reports.

        Args:
            input_report_size: Input report length
            output_report_size: Output report length
            feature_report_size: Feature report length
        """
        if not self._handle:
            return
        self._input_report_size = input_report_size
        self._output_report_size = output_report_size
        self._feature_report_size = feature_report_size
        _transport_lib.transport_set_reportSize(
            self._handle, input_report_size, output_report_size, feature_report_size
        )

    # ========== Error Handling ==========

    def get_last_error(self) -> str:
        """
        Get the last raw HID error message.

        Returns:
            str: Error message string
        """
        if not self._handle:
            return ""

        buffer_size = 256
        buffer = ctypes.create_unicode_buffer(buffer_size)
        length = c_size_t(buffer_size)
        result = _transport_lib.transport_raw_hid_last_error(
            self._handle, ctypes.cast(buffer, ctypes.c_void_p), ctypes.byref(length)
        )
        if result != 0:
            return ""
        return buffer.value

    def get_last_error_info(self) -> dict:
        """
        Get detailed error information from the transport library.

        Returns:
            dict: Error information containing error_code, error_message, function_name, timestamp, and line_number
        """
        if not self._handle:
            return {}

        # Define TransportErrorInfo structure
        class TransportErrorInfo(ctypes.Structure):
            _fields_ = [
                ("error_code", c_uint32),
                ("error_message", c_char * 256),
                ("function_name", c_char * 64),
                ("timestamp", c_uint32),
                ("line_number", c_uint32),
            ]

        error_info = TransportErrorInfo()
        result = _transport_lib.transport_get_last_error_info(
            self._handle, ctypes.byref(error_info)
        )

        if result == 0:  # TRANSPORT_SUCCESS is 0
            return {
                "error_code": error_info.error_code,
                "error_message": error_info.error_message.decode(
                    "utf-8", errors="ignore"
                ),
                "function_name": error_info.function_name.decode(
                    "utf-8", errors="ignore"
                ),
                "timestamp": error_info.timestamp,
                "line_number": error_info.line_number,
            }
        return {}

    # ========== Static Methods ==========

    @staticmethod
    def disable_output(disable: bool = True) -> None:
        """
        Globally disable lower-level output (e.g., debug logs).

        Args:
            disable: Whether to disable output
        """
        _transport_lib.transport_disable_output(1 if disable else 0)

    @staticmethod
    def create_device_info_from_dict(device_dict: dict) -> _HidDeviceInfo:
        """
        Create a _HidDeviceInfo structure from a device dictionary.

        Args:
            device_dict: Device information dictionary

        Returns:
            _HidDeviceInfo structure
        """
        device_info = _HidDeviceInfo()
        path = device_dict.get("path", "")
        device_info.path = path.encode("utf-8") if isinstance(path, str) else path
        device_info.vendor_id = device_dict.get("vendor_id", 0)
        device_info.product_id = device_dict.get("product_id", 0)
        device_info.serial_number = device_dict.get("serial_number", "")
        device_info.release_number = device_dict.get("release_number", 0)
        device_info.manufacturer_string = device_dict.get("manufacturer_string", "")
        device_info.product_string = device_dict.get("product_string", "")
        device_info.usage_page = device_dict.get("usage_page", 0)
        device_info.usage = device_dict.get("usage", 0)
        device_info.interface_number = device_dict.get("interface_number", 0)
        device_info.next = None
        return device_info

    @staticmethod
    def enumerate_devices(vendor_id: int, product_id: int) -> List[dict]:
        """
        Enumerate HID devices matching the given vendor and product IDs.

        使用C库内置的hidapi进行枚举，避免与Python的hidapi包冲突。

        Args:
            vendor_id: USB vendor ID
            product_id: USB product ID

        Returns:
            List of device information dictionaries
        """
        device_list = []
        # Use C library's hidapi to avoid conflicts
        dev_info_ptr = _transport_lib.transport_hid_enumerate(vendor_id, product_id)

        if not dev_info_ptr:
            return device_list

        try:
            current = dev_info_ptr
            while current:
                info = current.contents

                # Only include interface 0 to avoid duplicates
                if info.interface_number == 0:
                    device_list.append(
                        {
                            "path": info.path.decode("utf-8") if info.path else "",
                            "vendor_id": info.vendor_id,
                            "product_id": info.product_id,
                            "serial_number": (
                                info.serial_number if info.serial_number else ""
                            ),
                            "manufacturer_string": (
                                info.manufacturer_string
                                if info.manufacturer_string
                                else ""
                            ),
                            "product_string": (
                                info.product_string if info.product_string else ""
                            ),
                            "release_number": info.release_number,
                            "usage_page": info.usage_page,
                            "usage": info.usage,
                            "interface_number": info.interface_number,
                        }
                    )

                current = info.next
        finally:
            # Free the enumeration list
            _transport_lib.transport_hid_free_enumeration(dev_info_ptr)

        return device_list

    # ========== Properties ==========

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

    # ========== Legacy Method Aliases (for backward compatibility) ==========

    def getFirmwareVersion(self) -> str:
        """Legacy alias for get_firmware_version()."""
        return self.get_firmware_version()

    def clearTaskQueue(self) -> None:
        """Legacy alias for clear_task_queue()."""
        self.clear_task_queue()

    def wakeScreen(self) -> None:
        """Legacy alias for wakeup_screen()."""
        self.wakeup_screen()

    def keyClear(self, index: int) -> None:
        """Legacy alias for clear_key()."""
        self.clear_key(index)

    def keyAllClear(self) -> None:
        """Legacy alias for clear_all_keys()."""
        self.clear_all_keys()

    def switchMode(self, mode: int) -> None:
        """Legacy alias for change_mode()."""
        self.change_mode(mode)

    def open(self, device_path: bytes) -> bool:
        """
        Open a device connection using the device path.

        Args:
            device_path: Device path as bytes
        """
        if self._is_open or self._handle is not None:
            # Already opened
            print("[WARNING] Device already open", flush=True)
            return False

        # Create device info structure from path
        device_info = _HidDeviceInfo()
        device_info.path = device_path

        if self._device_info:
            # Use stored device info for other fields
            device_info.vendor_id = self._device_info.vendor_id
            device_info.product_id = self._device_info.product_id
            device_info.serial_number = self._device_info.serial_number
            device_info.release_number = self._device_info.release_number
            device_info.manufacturer_string = self._device_info.manufacturer_string
            device_info.product_string = self._device_info.product_string
            device_info.usage_page = self._device_info.usage_page
            device_info.usage = self._device_info.usage
            device_info.interface_number = self._device_info.interface_number

        # Create the transport handle
        handle_ptr = c_void_p()
        result = _transport_lib.transport_create(
            ctypes.byref(device_info), ctypes.byref(handle_ptr)
        )
        if result != 0:  # TRANSPORT_SUCCESS is 0
            print(f"[ERROR] Failed to create transport handle: {result}", flush=True)
            return False
        self._handle = handle_ptr.value
        self._is_open = True
        return True

    def close(self) -> None:
        """Close the device connection and release resources."""
        # CRITICAL: Ensure clean shutdown even if called multiple times
        if not self._is_open and not self._handle:
            return

        if self._handle:
            try:
                _transport_lib.transport_destroy(self._handle)
            except Exception as e:
                print(f"[ERROR] Failed to destroy transport: {e}", flush=True)
            finally:
                self._handle = None
                self._is_open = False

    def read_(self, size: int) -> Optional[bytes]:
        """
        Read data from the device with specified size.

        Args:
            size: Number of bytes to read

        Returns:
            bytes: Data read from device, or None if error occurred
        """
        if not self._handle:
            return None

        try:
            # CRITICAL: Allocate buffer and prepare for C call
            buffer = (c_uint8 * size)()
            length = c_size_t(size)

            # Store handle locally to avoid attribute access during C call
            handle = self._handle

            # CRITICAL FOR LINUX: Release GIL before blocking C call
            # This prevents deadlocks when C library blocks on I/O
            import threading

            gil_state = None
            try:
                # Call C function - ctypes should handle GIL automatically
                # but we ensure thread safety by using local variables
                result = _transport_lib.transport_read(
                    handle,
                    buffer,
                    ctypes.byref(length),
                    100,  # 使用 100ms 超时用于轮询，避免长时间阻塞
                )
            finally:
                # GIL is automatically reacquired by ctypes
                pass

            # 检查结果，0表示成功，非0表示错误
            if result == 0 and length.value > 0:
                # CRITICAL: Use simple bytes() constructor for safer conversion
                # ctypes.string_at can cause issues in multi-threaded environments on Linux
                data_length = int(length.value)
                # Create bytes directly from buffer slice
                data_bytes = bytes(buffer[:data_length])
                return data_bytes
            else:
                # 超时或无数据是正常的（设备没有事件时），返回None
                return None
        except Exception as e:
            # 捕获所有可能的异常，避免线程崩溃
            import traceback

            print(f"read_ exception: {e}", flush=True)
            traceback.print_exc()
            return None

    # ========== Legacy Image Methods (DualDevice support) ==========

    def setBackgroundImg(self, buffer: bytes, size: int) -> None:
        """
        Legacy method: Set background image from buffer.

        Args:
            buffer: Image data buffer
            size: Size of the buffer
        """
        self.set_background_bitmap(buffer[:size])

    def setBackgroundImgDualDevice(self, path) -> None:
        """
        Legacy method: Set background image from file path (for dual device).

        Args:
            path: Path to the image file (can be str, bytes, c_char_p, or os.PathLike)
        """
        try:
            # Convert c_char_p to string if needed
            if isinstance(path, c_char_p):
                path = (
                    path.value.decode("utf-8")
                    if isinstance(path.value, bytes)
                    else path.value
                )
            elif isinstance(path, bytes):
                path = path.decode("utf-8")

            if path is None:
                raise ValueError("Path cannot be None")

            with open(path, "rb") as f:
                jpeg_data = f.read()
            self.set_background_image_stream(jpeg_data)
        except Exception as e:
            raise RuntimeError(f"Failed to load image from {path}: {e}")

    def setKeyImg(self, path, key: int) -> None:
        """
        Legacy method: Set key image from file path.

        Args:
            path: Path to the image file (can be str, bytes, c_char_p, or os.PathLike)
            key: Key index
        """
        try:
            # Convert c_char_p to string if needed
            if isinstance(path, c_char_p):
                path = (
                    path.value.decode("utf-8")
                    if isinstance(path.value, bytes)
                    else path.value
                )
            elif isinstance(path, bytes):
                path = path.decode("utf-8")

            if path is None:
                raise ValueError("Path cannot be None")

            with open(path, "rb") as f:
                jpeg_data = f.read()
            res = self.set_key_image_stream(jpeg_data, key)
            return res
        except Exception as e:
            raise RuntimeError(f"Failed to load image from {path}: {e}")

    def setKeyImgDualDevice(self, path, key: int) -> None:
        """
        Legacy method: Set key image from file path (for dual device).

        Args:
            path: Path to the image file (can be str, bytes, c_char_p, or os.PathLike)
            key: Key index
        """
        return self.setKeyImg(path, key)

    def setKeyImgDataDualDevice(self, data: bytes, key: int) -> None:
        """
        Legacy method: Set key image from data buffer (for dual device).

        Args:
            data: Image data as bytes
            key: Key index
        """
        self.set_key_image_stream(data, key)

    def setBrightness(self, percent: int) -> None:
        """
        Legacy method: Set brightness.

        Args:
            percent: Brightness percentage (0-100)
        """
        self.set_key_brightness(percent)

    def disconnected(self) -> None:
        """Legacy method: Notify device of disconnection."""
        self.notify_disconnected()

    def refresh(self) -> None:
        """Legacy method: Refresh the display."""
        self.refresh_screen()
