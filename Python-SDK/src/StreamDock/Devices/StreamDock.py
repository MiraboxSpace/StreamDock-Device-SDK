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
    """自定义异常类型，用于传输错误"""

    def __init__(self, message, code=None):
        super().__init__(message)
        self.code = code  # 可选的错误代码

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

    # 打开设备
    def open(self):
        res1 = self.transport.open(bytes(self.path, "utf-8"))
        self._setup_reader(self._read)
        # macOS need to get firmware version after opening device
        if platform.system() == "Darwin":
            self.firmware_version = self.transport.get_firmware_version()
        return res1

    # 初始化
    def init(self):
        self.set_device()
        self.wakeScreen()
        self.set_brightness(100)
        self.clearAllIcon()
        if platform.system() != "Darwin":
            self.firmware_version = self.transport.get_firmware_version()
        self.refresh()

    # 设置设备参数
    @abstractmethod
    def set_device(self):
        pass

    # 设置设备的LED亮度
    def set_led_brightness(self, percent):
        if self.feature_option.hasRGBLed:
            return self.transport.set_led_brightness(percent)

    # 设置设备的LED颜色
    def set_led_color(self, r, g, b):
        if self.feature_option.hasRGBLed:
            return self.transport.set_led_color(self.feature_option.ledCounts, r, g, b)

    # 重置设备的LED效果
    def reset_led_effect(self):
        if self.feature_option.hasRGBLed:
            return self.transport.reset_led_color()

    # 关闭设备
    def close(self):
        """
        关闭设备并释放所有资源。

        CRITICAL: This method must be called before the object is destroyed to ensure
        clean shutdown of the C library and prevent segmentation faults.
        """
        # print(f"[DEBUG] 关闭设备: {self.path}")

        # CRITICAL: Stop reader thread first and wait for it to finish
        self.run_read_thread = False

        if self.read_thread and self.read_thread.is_alive():
            try:
                # Give thread time to exit naturally
                self.read_thread.join(timeout=2.0)  # 等待最多2秒
                if self.read_thread.is_alive():
                    print(f"[WARNING] 读取线程未能及时退出", flush=True)
            except Exception as e:
                print(f"[WARNING] 等待读取线程退出时出错: {e}", flush=True)

        # 发送断开连接命令 (may fail if device already disconnected)
        try:
            self.disconnected()
        except Exception as e:
            print(f"[WARNING] 发送断开命令时出错: {e}", flush=True)

        # CRITICAL: Close transport properly to release HID device
        try:
            self.transport.close()
        except Exception as e:
            print(f"[WARNING] 关闭transport时出错: {e}", flush=True)

        # Clear callback to break any circular references
        with self._callback_lock:
            self.key_callback = None

        # print(f"[DEBUG] 设备已关闭")

    # 断开连接清楚所有显示
    def disconnected(self):
        self.transport.disconnected()

    # 清除某个按键的图标
    def clearIcon(self, index):
        origin = index
        if origin not in range(1, self.KEY_COUNT + 1):
            print(f"key '{origin}' out of range. you should set (1 ~ {self.KEY_COUNT})")
            return -1
        logical_key = ButtonKey(origin) if isinstance(origin, int) else origin
        hardware_key = self.get_image_key(logical_key)
        self.transport.keyClear(hardware_key)

    # 清除所有按键的图标
    def clearAllIcon(self):
        self.transport.keyAllClear()

    # 唤醒屏幕
    def wakeScreen(self):
        self.transport.wakeScreen()

    # 刷新设备显示
    def refresh(self):
        self.transport.refresh()

    # 获取设备路径
    def getPath(self):
        return self.path

    # 获取设备反馈的信息
    def read(self):
        """
        :argtypes:存放信息的字节数组, 字节数组的长度建议1024

        """
        data = self.transport.read_(1024)
        return data

    # 一直检测设备有无信息反馈，建议开线程使用
    def whileread(self):
        """
        @deprecated 建议使用内置的异步回调机制，而不是直接调用此方法
        """
        from ..InputTypes import EventType
        while 1:
            try:
                data = self.read()
                if data != None and len(data) >= 11:
                    try:
                        event = self.decode_input_event(data[9], data[10])
                        if event.event_type == EventType.BUTTON:
                            action = "按下" if event.state == 1 else "抬起"
                            print(f"按键{event.key.value if event.key else '?'}被{action}")
                        elif event.event_type == EventType.KNOB_ROTATE:
                            print(f"旋钮{event.knob_id.value if event.knob_id else '?'}向{event.direction.value if event.direction else '?'}旋转")
                        elif event.event_type == EventType.KNOB_PRESS:
                            action = "按下" if event.state == 1 else "抬起"
                            print(f"旋钮{event.knob_id.value if event.knob_id else '?'}被{action}")
                        elif event.event_type == EventType.SWIPE:
                            print(f"滑动手势: {event.direction.value if event.direction else '?'}")
                    except Exception:
                        pass
                # self.transport.deleteRead()
            except Exception as e:
                print("发生错误：")
                traceback.print_exc()  # 打印详细的异常信息
                break

    # #息屏
    # def screen_Off(self):
    #     res=self.transport.screen_Off()
    #     self.reset_Countdown(self.__seconds)
    #     return res
    # #唤醒屏幕
    # def screen_On(self):
    #     return self.transport.screen_On()
    # # 设置定时器时间
    # def set_seconds(self, data):
    #     self.__seconds = data
    #     self.reset_Countdown(self.__seconds)

    # # 重启定时器
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
    def set_key_image(self, key, image):
        pass

    # @abstractmethod
    # def set_key_imageData(self, key, image, width=126, height=126):
    #     pass

    @abstractmethod
    def set_brightness(self, percent):
        pass

    @abstractmethod
    def set_touchscreen_image(self, path):
        pass

    @abstractmethod
    def get_image_key(self, logical_key: ButtonKey) -> int:
        """
        将逻辑键值转换为硬件键值（用于设置图片）

        Args:
            logical_key: 逻辑键值枚举

        Returns:
            int: 硬件键值
        """
        pass

    @abstractmethod
    def decode_input_event(self, hardware_code: int, state: int) -> InputEvent:
        """
        将硬件事件码解码为统一的 InputEvent

        Args:
            hardware_code: 硬件事件码
            state: 状态 (0=释放, 1=按下)

        Returns:
            InputEvent: 解码后的事件对象
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
                            # 写入成功确认
                            pass
                        else:
                            try:
                                # 使用设备类的事件解码器
                                if self.feature_option.deviceType!=device_type.k1pro:
                                    event = self.decode_input_event(arr[9], arr[10])
                                else:
                                    event = self.decode_input_event(arr[10], arr[11])
                                # Get callback reference with lock
                                with self._callback_lock:
                                    callback = self.key_callback

                                # Call callback OUTSIDE of lock to avoid deadlocks
                                if callback is not None:
                                    try:
                                        # 回调签名: callback(device, event)
                                        callback(self, event)
                                    except Exception as callback_error:
                                        print(
                                            f"按键回调函数发生错误: {callback_error}",
                                            flush=True,
                                        )
                                        traceback.print_exc()
                            except Exception as decode_error:
                                print(
                                    f"事件解码错误: {decode_error}", flush=True
                                )
                                traceback.print_exc()
                    # else:
                    #     print("read control", arr)
                    # Don't explicitly delete arr - let Python's GC handle it
                    # del arr causes issues with ctypes buffers on Linux

                except Exception as e:
                    print(f"读取数据时发生错误: {e}", flush=True)
                    traceback.print_exc()
                    continue
        except Exception as outer_error:
            print(f"[FATAL] 读取线程外层异常: {outer_error}", flush=True)
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
                    print(f"按键 {event.key.value} 被按下")
                elif event.event_type == EventType.KNOB_ROTATE:
                    print(f"旋钮旋转")
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
