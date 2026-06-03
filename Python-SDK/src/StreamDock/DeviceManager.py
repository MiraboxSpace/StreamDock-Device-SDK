import platform
import threading
import time
from typing import Callable, Optional
# import pywinusb.hid as hid
from .ProductIDs import USBVendorIDs, USBProductIDs, g_products
from .Transport.LibUSBHIDAPI import LibUSBHIDAPI

# Platform-specific imports
if platform.system() == "Linux":
    import pyudev
elif platform.system() == "Windows":
    try:
        import wmi
        import pythoncom
        WINDOWS_SUPPORT = True
    except ImportError:
        print("Warning: wmi module not installed, using polling mode")
        WINDOWS_SUPPORT = False
elif platform.system() == "Darwin":
    # macOS specific imports can be added here if needed
    pass

class DeviceManager:
    @staticmethod
    def _get_transport(transport):
        return LibUSBHIDAPI()

    def __init__(self, transport=None):
        self.transport = self._get_transport(transport)
        self.streamdocks = []
        self._device_lock = threading.RLock()
        self._on_device_added = None
        self._on_device_removed = None
        self._auto_open = True
        self._auto_init = False

    def enumerate(self) -> list:
        # CRITICAL: Clear old list to avoid stale references
        with self._device_lock:
            self.streamdocks.clear()

            products = g_products
            for vid, pid, class_type in products:
                found_devices = self.transport.enumerate_devices(
                    vendor_id=vid, product_id=pid
                )
                for d in found_devices:
                    self.streamdocks.append(self._create_device(class_type, d))
            return self.streamdocks

    def set_device_change_callback(
        self,
        on_device_added: Optional[Callable] = None,
        on_device_removed: Optional[Callable] = None,
    ):
        """
        Register callbacks for hotplug events.

        on_device_added(device) is called after the device is added to the manager
        and after optional auto open/init has completed.

        on_device_removed(device) is called before the device is closed and after
        it is removed from the manager.
        """
        self._on_device_added = on_device_added
        self._on_device_removed = on_device_removed

    def listen(
        self,
        on_device_added: Optional[Callable] = None,
        on_device_removed: Optional[Callable] = None,
        auto_open: bool = True,
        auto_init: bool = False,
    ):
        """
        Listen for device hotplug events, cross-platform.

        Args:
            on_device_added: Optional callback called with the new device.
            on_device_removed: Optional callback called with the removed device.
            auto_open: Open newly attached devices automatically.
            auto_init: Initialize newly attached devices after opening.
        """
        if on_device_added is not None or on_device_removed is not None:
            self.set_device_change_callback(on_device_added, on_device_removed)
        self._auto_open = auto_open
        self._auto_init = auto_init

        products = g_products
        system = platform.system()

        if system == "Linux":
            self._listen_linux(products)
        elif system == "Windows":
            self._listen_windows(products)
        elif system == "Darwin":
            self._listen_macos(products)
        else:
            print(f"Unsupported operating system: {system}")

    def _create_device(self, class_type, device_info):
        device_info_struct = LibUSBHIDAPI.create_device_info_from_dict(device_info)
        device_transport = LibUSBHIDAPI(device_info_struct)
        return class_type(device_transport, device_info)

    def _device_exists(self, device_path):
        return any(device.getPath() == device_path for device in self.streamdocks)

    def _safe_callback(self, callback, device, event_name):
        if callback is None:
            return
        try:
            callback(device)
        except Exception as e:
            print(f"{event_name} callback error: {e}", flush=True)

    def _open_hotplug_device(self, device):
        if not self._auto_open:
            return
        device.open()
        if self._auto_init:
            device.init()

    def _add_device(self, class_type, device_info):
        device_path = device_info["path"]

        with self._device_lock:
            if self._device_exists(device_path):
                return None
            new_device = self._create_device(class_type, device_info)
            self.streamdocks.append(new_device)

        try:
            self._open_hotplug_device(new_device)
        except Exception as e:
            print(
                f"[WARNING] Failed to open hotplug device {device_path}: {e}",
                flush=True,
            )

        self._safe_callback(self._on_device_added, new_device, "Device added")
        return new_device

    def _remove_device_by_path(self, device_path):
        removed_devices = []
        with self._device_lock:
            for device in list(self.streamdocks):
                if device.getPath() == device_path:
                    self.streamdocks.remove(device)
                    removed_devices.append(device)

        for device in removed_devices:
            self._safe_callback(self._on_device_removed, device, "Device removed")
            try:
                device.close(notify=False)
            except Exception as e:
                print(
                    f"[WARNING] Error closing removed device {device.getPath()}: {e}",
                    flush=True,
                )
        return removed_devices

    def _add_missing_devices(self, products):
        added_devices = []
        for vid, pid, class_type in products:
            found_devices = self.transport.enumerate_devices(vendor_id=vid, product_id=pid)
            for device_info in found_devices:
                if not self._device_exists(device_info["path"]):
                    print(f"[add] path: {device_info['path']}")
                    added_device = self._add_device(class_type, device_info)
                    if added_device is not None:
                        added_devices.append(added_device)
        return added_devices

    def _current_device_paths(self, products):
        current_paths = set()
        for vid, pid, _ in products:
            found_devices = self.transport.enumerate_devices(vendor_id=vid, product_id=pid)
            for device_info in found_devices:
                current_paths.add(device_info["path"])
        return current_paths

    def _remove_missing_devices(self, products):
        current_paths = self._current_device_paths(products)

        with self._device_lock:
            devices_to_remove = [
                device for device in self.streamdocks if device.getPath() not in current_paths
            ]

        for device in devices_to_remove:
            print(f"[remove] path: {device.getPath()}")
            self._remove_device_by_path(device.getPath())

        return devices_to_remove

    def _listen_linux(self, products):
        """Linux uses pyudev to listen for device events"""
        context = pyudev.Context()
        monitor = pyudev.Monitor.from_netlink(context)
        monitor.filter_by(subsystem="usb")

        while True:
            try:
                device = monitor.poll(timeout=1)
                if device is None:
                    self._remove_missing_devices(products)
                    self._add_missing_devices(products)
                    continue
                self._handle_device_event(device.action, device, products)
            except Exception as e:
                print(f"Linux device listener error: {e}", flush=True)

    def _listen_windows(self, products):
        """Windows uses WMI to listen for device events"""
        if not WINDOWS_SUPPORT:
            print("WMI unavailable, using polling mode")
            self._fallback_polling(products)
            return

        try:
            pythoncom.CoInitialize()
            c = wmi.WMI()
            watcher = c.Win32_DeviceChangeEvent.watch_for()

            while True:
                try:
                    event = watcher()
                    if event.EventType == 2:  # Device connected
                        self._check_new_devices_windows(products)
                    elif event.EventType == 3:  # Device disconnected
                        self._check_removed_devices_windows(products)
                except Exception as e:
                    print(f"Windows device listener error: {e}")
                    time.sleep(1)
        except Exception as e:
            print(f"Windows WMI initialization failed: {e}")
            self._fallback_polling(products)
        finally:
            pythoncom.CoUninitialize()

    def _listen_macos(self, products):
        """macOS uses polling to listen for device events"""
        self._fallback_polling(products)

    def _fallback_polling(self, products):
        """Fall back to polling mode for systems without real-time monitoring"""
        current_devices = self._current_device_paths(products)

        while True:
            try:
                new_devices = self._current_device_paths(products)

                added_devices = new_devices - current_devices
                for device_path in added_devices:
                    print(f"[add] path: {device_path}")
                    self._handle_device_addition(device_path, products)

                removed_devices = current_devices - new_devices
                for device_path in removed_devices:
                    print(f"[remove] path: {device_path}")
                    self._remove_device_by_path(device_path)

                current_devices = new_devices
                time.sleep(2)
            except Exception as e:
                print(f"Polling listener error: {e}")
                time.sleep(5)

    def _handle_device_event(self, action, device, products):
        """Handle device events (Linux)"""
        if action not in ["add", "remove"]:
            return

        if action == "remove":
            for _ in range(3):
                removed_devices = self._remove_missing_devices(products)
                if removed_devices:
                    break
                time.sleep(0.2)
            return

        for _ in range(10):
            added_devices = self._add_missing_devices(products)
            if added_devices:
                break
            time.sleep(0.2)

    def _check_new_devices_windows(self, products):
        """Check for new devices on Windows"""
        self._add_missing_devices(products)

    def _check_removed_devices_windows(self, products):
        """Check for removed devices on Windows"""
        self._remove_missing_devices(products)

    def _handle_device_addition(self, device_path, products):
        """Handle device addition events (polling mode)"""
        for vid, pid, class_type in products:
            found_devices = self.transport.enumerate_devices(vendor_id=vid, product_id=pid)
            for device_info in found_devices:
                if device_info["path"] == device_path:
                    self._add_device(class_type, device_info)
                    return

