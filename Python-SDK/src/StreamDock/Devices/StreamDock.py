import platform
import threading
from abc import ABC, ABCMeta, abstractmethod
import threading
from abc import ABC, ABCMeta, abstractmethod
import ctypes
import ctypes.util
import threading
import traceback
from typing import Optional

from ..FeatrueOption import FeatrueOption, device_type
from ..Transport.LibUSBHIDAPI import LibUSBHIDAPI
from ..InputTypes import InputEvent, ButtonKey


class TransportError(Exception):
    """Custom exception type for transport errors"""

    def __init__(self, message, code=None):
        super().__init__(message)
        self.code = code  # Optional error code

    def __str__(self):
        if self.code:
            return f"[Error Code {self.code}] {super().__str__()}"
        return super().__str__()


class StreamDock(ABC):
    """
    Represents a physically attached StreamDock device.
    """

    KEY_COUNT = 0
    KEY_COLS = 0
    KEY_ROWS = 0

    KEY_PIXEL_WIDTH = 0
    KEY_PIXEL_HEIGHT = 0
    KEY_IMAGE_FORMAT = ""
    KEY_FLIP = (False, False)
    KEY_ROTATION = 0
    KEY_MAP = False

    TOUCHSCREEN_PIXEL_WIDTH = 0
    TOUCHSCREEN_PIXEL_HEIGHT = 0
    TOUCHSCREEN_IMAGE_FORMAT = ""
    TOUCHSCREEN_FLIP = (False, False)
    TOUCHSCREEN_ROTATION = 0

    DIAL_COUNT = 0

    DECK_TYPE = ""
    DECK_VISUAL = False
    DECK_TOUCH = False

    transport: LibUSBHIDAPI
    screenlicent = None
    __metaclass__ = ABCMeta
    __seconds = 300

    feature_option: FeatrueOption

    def __init__(self, transport1: LibUSBHIDAPI, devInfo):
        self.transport = transport1
        self.vendor_id = devInfo["vendor_id"]
        self.product_id = devInfo["product_id"]
        self.path = devInfo["path"]
        self.serial_number = devInfo.get("serial_number", "")
        self.firmware_version = ""
        self.read_thread = None
        self.run_read_thread = False
        self.feature_option = FeatrueOption()
        self.key_callback = None
        # CRITICAL: Add lock to protect callback access in multi-threaded environment
        self._callback_lock = threading.Lock()

        # self.update_lock = threading.RLock()
        # self.screenlicent=threading.Timer(self.__seconds,self.screen_Off)
        # self.screenlicent.start()

    def __del__(self):
        """
        Delete handler for the StreamDock, automatically closing the transport
        if it is currently open and terminating the transport reader thread.

        CRITICAL: This is called during garbage collection which may happen during
        interpreter shutdown. We need to be extremely careful to avoid calling
        C code during shutdown as it can cause segmentation faults.
        """
        import sys

        # CRITICAL: Don't call C code during interpreter shutdown
        if sys.is_finalizing():
            # During interpreter shutdown, skip cleanup to avoid segfault
            # The OS will clean up resources when process exits
            return

        try:
            # Stop the reader thread (safe operation)
            self.run_read_thread = False
            if self.read_thread and self.read_thread.is_alive():
                self.read_thread.join(timeout=0.5)  # Short timeout during __del__
        except (TransportError, ValueError, RuntimeError):
            pass

        try:
            # Close transport - this may call C code but we checked is_finalizing above
            self.close()
        except TransportError:
            pass

    def __enter__(self):
        """
        Enter handler for the StreamDock, taking the exclusive update lock on
        the deck. This can be used in a `with` statement to ensure that only one
        thread is currently updating the deck, even if it is doing multiple
        operations (e.g. setting the image on multiple keys).
        """
        # self.update_lock.acquire()

    def __exit__(self, type, value, traceback):
        """
        Exit handler for the StreamDock, releasing the exclusive update lock on
        the deck.
        """
        # self.update_lock.release()

    # Open device
    def open(self):
        res1 = self.transport.open(bytes(self.path, "utf-8"))
        self._setup_reader(self._read)
        # macOS need to get firmware version after opening device
        if platform.system() == "Darwin":
            self.firmware_version = self.transport.get_firmware_version()
        return res1

    # Initialize
    def init(self):
        self.set_device()
        self.wakeScreen()
        self.set_brightness(100)
        self.clearAllIcon()
        if platform.system() != "Darwin":
            self.firmware_version = self.transport.get_firmware_version()
        self.refresh()

    # Set device parameters
    @abstractmethod
    def set_device(self):
        pass

    # Set device LED brightness
    def set_led_brightness(self, percent):
        if self.feature_option.hasRGBLed:
            return self.transport.set_led_brightness(percent)

    # Set device LED color
    def set_led_color(self, r, g, b):
        if self.feature_option.hasRGBLed:
            return self.transport.set_led_color(self.feature_option.ledCounts, r, g, b)

    # Reset device LED effects
    def reset_led_effect(self):
        if self.feature_option.hasRGBLed:
            return self.transport.reset_led_color()

    # Close device
    def close(self):
        """
        Close the device and release all resources.

        CRITICAL: This method must be called before the object is destroyed to ensure
        clean shutdown of the C library and prevent segmentation faults.
        """
        # print(f"[DEBUG] Closing device: {self.path}")

        # CRITICAL: Stop reader thread first and wait for it to finish
        self.run_read_thread = False

        if self.read_thread and self.read_thread.is_alive():
            try:
                # Give thread time to exit naturally
                self.read_thread.join(timeout=2.0)  # Wait up to 2 seconds
                if self.read_thread.is_alive():
                    print("[WARNING] Read thread did not exit in time", flush=True)
            except Exception as e:
                print(f"[WARNING] Error while waiting for read thread to exit: {e}", flush=True)

        # Send disconnect command (may fail if device already disconnected)
        try:
            self.disconnected()
        except Exception as e:
            print(f"[WARNING] Error sending disconnect command: {e}", flush=True)

        # CRITICAL: Close transport properly to release HID device
        try:
            self.transport.close()
        except Exception as e:
            print(f"[WARNING] Error closing transport: {e}", flush=True)

        # Clear callback to break any circular references
        with self._callback_lock:
            self.key_callback = None

        # print("[DEBUG] Device closed")

    # Disconnect and clear all displays
    def disconnected(self):
        self.transport.disconnected()

    # Clear a specific key icon
    def clearIcon(self, index):
        origin = index
        if origin not in range(1, self.KEY_COUNT + 1):
            print(f"key '{origin}' out of range. you should set (1 ~ {self.KEY_COUNT})")
            return -1
        logical_key = ButtonKey(origin) if isinstance(origin, int) else origin
        hardware_key = self.get_image_key(logical_key)
        self.transport.keyClear(hardware_key)

    # Clear all key icons
    def clearAllIcon(self):
        self.transport.keyAllClear()

    # Wake the screen
    def wakeScreen(self):
        self.transport.wakeScreen()

    # Refresh the device display
    def refresh(self):
        self.transport.refresh()

    # Get device path
    def getPath(self):
        return self.path

    # Get device feedback data
    def read(self):
        """
        :argtypes: byte array to store info; recommended length 1024

        """
        data = self.transport.read_(1024)
        return data

    # Continuously check for device feedback; recommended to run in a thread
    def whileread(self):
        """
        @deprecated Use the built-in async callback mechanism instead of calling this directly
        """
        from ..InputTypes import EventType

        while 1:
            try:
                data = self.read()
                if data != None and len(data) >= 11:
                    try:
                        event = self.decode_input_event(data[9], data[10])
                        if event.event_type == EventType.BUTTON:
                            action = "pressed" if event.state == 1 else "released"
                            print(
                                f"Key {event.key.value if event.key else '?'} was {action}"
                            )
                        elif event.event_type == EventType.KNOB_ROTATE:
                            print(
                                f"Knob {event.knob_id.value if event.knob_id else '?'} rotated {event.direction.value if event.direction else '?'}"
                            )
                        elif event.event_type == EventType.KNOB_PRESS:
                            action = "pressed" if event.state == 1 else "released"
                            print(
                                f"Knob {event.knob_id.value if event.knob_id else '?'} was {action}"
                            )
                        elif event.event_type == EventType.SWIPE:
                            print(
                                f"Swipe gesture: {event.direction.value if event.direction else '?'}"
                            )
                    except Exception:
                        pass
                # self.transport.deleteRead()
            except Exception as e:
                print("Error occurred:")
                traceback.print_exc()  # Print detailed exception info
                break

    # # Screen off
    # def screen_Off(self):
    #     res=self.transport.screen_Off()
    #     self.reset_Countdown(self.__seconds)
    #     return res
    # # Wake screen
    # def screen_On(self):
    #     return self.transport.screen_On()
    # # Set timer interval
    # def set_seconds(self, data):
    #     self.__seconds = data
    #     self.reset_Countdown(self.__seconds)

    # # Restart timer
    # def reset_Countdown(self, data):
    #     if self.screenlicent is not None:
    #         self.screenlicent.cancel()
    #     if hasattr(self, 'screen_Off'):
    #         self.screenlicent = threading.Timer(data, self.screen_Off)
    #         self.screenlicent.start()

    def get_serial_number(self):
        """Return the device serial number."""
        return self.serial_number


    @abstractmethod
    def set_key_image(self, key, path) -> int | None:
        pass

    # @abstractmethod
    # def set_key_imageData(self, key, image, width=126, height=126):
    #     pass

    @abstractmethod
    def set_brightness(self, percent):
        pass

    @abstractmethod
    def set_touchscreen_image(self, path) -> int | None:
        pass

    @abstractmethod
    def get_image_key(self, logical_key: ButtonKey) -> int:
        """
        Convert logical key value to hardware key value (for setting images)

        Args:
            logical_key: Logical key enum

        Returns:
            int: Hardware key value
        """
        pass

    @abstractmethod
    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        Decode hardware event codes into a unified InputEvent

        Args:
            hardware_code: Hardware event code
            state: State (0=release, 1=press)

        Returns:
            InputEvent: Decoded event object
        """
        pass

    def id(self):
        """
        Retrieves the physical ID of the attached StreamDock. This can be used
        to differentiate one StreamDock from another.

        :rtype: str
        :return: Identifier for the attached device.
        """
        return self.getPath()

    def _read(self):
        try:
            while self.run_read_thread:
                try:
                    arr = self.read()
                    if arr is not None and len(arr) >= 10:
                        if arr[9] == 0xFF:
                            # Confirm write success
                            pass
                        else:
                            try:
                                # Use the device class event decoder
                                if self.feature_option.deviceType != device_type.k1pro:
                                    event = self.decode_input_event(arr[9], arr[10])
                                else:
                                    event = self.decode_input_event(arr[10], arr[11])
                                # Get callback reference with lock
                                with self._callback_lock:
                                    callback = self.key_callback

                                # Call callback OUTSIDE of lock to avoid deadlocks
                                if callback is not None:
                                    try:
                                        # Callback signature: callback(device, event)
                                        callback(self, event)
                                    except Exception as callback_error:
                                        print(
                                            f"Key callback error: {callback_error}",
                                            flush=True,
                                        )
                                        traceback.print_exc()
                            except Exception as decode_error:
                                print(f"Event decode error: {decode_error}", flush=True)
                                traceback.print_exc()
                    # else:
                    #     print("read control", arr)
                    # Don't explicitly delete arr - let Python's GC handle it
                    # del arr causes issues with ctypes buffers on Linux

                except Exception as e:
                    print(f"Error reading data: {e}", flush=True)
                    traceback.print_exc()
                    continue
        except Exception as outer_error:
            print(f"[FATAL] Read thread outer exception: {outer_error}", flush=True)
            traceback.print_exc()
        finally:
            pass

    def _setup_reader(self, callback):
        """
        Sets up the internal transport reader thread with the given callback,
        for asynchronous processing of HID events from the device. If the thread
        already exists, it is terminated and restarted with the new callback
        function.

        :param function callback: Callback to run on the reader thread.
        """
        if self.read_thread is not None:
            self.run_read_thread = False
            try:
                self.read_thread.join()
                # return
            except RuntimeError:
                pass

        if callback is not None:
            self.run_read_thread = True
            self.read_thread = threading.Thread(target=callback)
            self.read_thread.daemon = True
            self.read_thread.start()

    def set_key_callback(self, callback):
        """
        Sets the callback function called each time a button on the StreamDock
        changes state (either pressed, or released), or a knob is rotated/pressed,
        or a swipe gesture is detected.

        .. note:: This callback will be fired from an internal reader thread.
                  Ensure that the given callback function is thread-safe.

        .. note:: Only one callback can be registered at one time.

        .. seealso:: See :func:`~StreamDock.set_key_callback_async` method for
                     a version compatible with Python 3 `asyncio` asynchronous
                     functions.

        :param function callback: Callback function with signature:
                                callback(device: StreamDock, event: InputEvent)

        Example:
            def on_input(device, event):
                from StreamDock.InputTypes import EventType
                if event.event_type == EventType.BUTTON:
                    print(f"Key {event.key.value} pressed")
                elif event.event_type == EventType.KNOB_ROTATE:
                    print("Knob rotated")
        """
        with self._callback_lock:
            self.key_callback = callback

    def set_key_callback_async(self, async_callback, loop=None):
        """
        Sets the asynchronous callback function called each time a button on the
        StreamDock changes state (either pressed, or released), or a knob is
        rotated/pressed, or a swipe gesture is detected. The given callback
        should be compatible with Python 3's `asyncio` routines.

        .. note:: The asynchronous callback will be fired in a thread-safe
                  manner.

        .. note:: This will override the callback (if any) set by
                  :func:`~StreamDock.set_key_callback`.

        :param function async_callback: Asynchronous callback function with signature:
                                        async_callback(device: StreamDock, event: InputEvent)
        :param asyncio.loop loop: Asyncio loop to dispatch the callback into
        """
        import asyncio

        loop = loop or asyncio.get_event_loop()

        def callback(*args):
            asyncio.run_coroutine_threadsafe(async_callback(*args), loop)

        self.set_key_callback(callback)

    def set_touchscreen_callback(self, callback):
        """
        Sets the callback function called each time there is an interaction
        with a touchscreen on the StreamDock.

        .. note:: This callback will be fired from an internal reader thread.
                  Ensure that the given callback function is thread-safe.

        .. note:: Only one callback can be registered at one time.

        .. seealso:: See :func:`~StreamDock.set_touchscreen_callback_async`
                     method for a version compatible with Python 3 `asyncio`
                     asynchronous functions.

        :param function callback: Callback function to fire each time a button
                                state changes.
        """
        self.touchscreen_callback = callback

    def set_touchscreen_callback_async(self, async_callback, loop=None):
        """
        Sets the asynchronous callback function called each time there is an
        interaction with the touchscreen on the StreamDock. The given callback
        should be compatible with Python 3's `asyncio` routines.

        .. note:: The asynchronous callback will be fired in a thread-safe
                  manner.

        .. note:: This will override the callback (if any) set by
                  :func:`~StreamDock.set_touchscreen_callback`.

        :param function async_callback: Asynchronous callback function to fire
                                        each time a button state changes.
        :param asyncio.loop loop: Asyncio loop to dispatch the callback into
        """
        import asyncio

        loop = loop or asyncio.get_event_loop()

        def callback(*args):
            asyncio.run_coroutine_threadsafe(async_callback(*args), loop)

        self.set_touchscreen_callback(callback)
