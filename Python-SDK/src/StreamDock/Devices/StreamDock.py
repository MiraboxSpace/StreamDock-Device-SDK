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
from ..Transport.LibUSBHIDAPI import LibUSBHIDAPI


class TransportError(Exception):
    """自定义异常类型，用于传输错误"""

    def __init__(self, message, code=None):
        super().__init__(message)
        self.code = code  # 可选的错误代码

    def __str__(self):
        if self.code:
            return f"[Error Code {self.code}] {super().__str__()}"
        return super().__str__()


KEY_MAPPING = {
    1: 11,
    2: 12,
    3: 13,
    4: 14,
    5: 15,
    6: 6,
    7: 7,
    8: 8,
    9: 9,
    10: 10,
    11: 1,
    12: 2,
    13: 3,
    14: 4,
    15: 5,
}


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

    def __init__(self, transport1: LibUSBHIDAPI, devInfo):
        self.transport = transport1
        self.vendor_id = devInfo["vendor_id"]
        self.product_id = devInfo["product_id"]
        self.path = devInfo["path"]
        self.serial_number = devInfo.get("serial_number", "")
        self.firmware_version = ""
        self.read_thread = None
        self.run_read_thread = False

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
        """
        try:
            self._setup_reader(None)
        except (TransportError, ValueError):
            pass

        try:
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

    def key(self, k):
        if self.KEY_MAP:
            return KEY_MAPPING[k]
        else:
            return k

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
    
    # 关闭设备
    def close(self):
        # print(f"[DEBUG] 关闭设备: {self.path}")
        # CRITICAL: Stop reader thread first and wait for it to finish
        self.run_read_thread = False
        if self.read_thread and self.read_thread.is_alive():
            try:
                self.read_thread.join(timeout=2.0)  # 等待最多2秒
            except Exception as e:
                print(f"[WARNING] 等待读取线程退出时出错: {e}")

        # 发送断开连接命令
        try:
            self.disconnected()
        except Exception as e:
            print(f"[WARNING] 发送断开命令时出错: {e}")

        # CRITICAL: Close transport properly to release HID device
        try:
            self.transport.close()
        except Exception as e:
            print(f"[WARNING] 关闭transport时出错: {e}")
        
        # Clear callback to break any circular references
        with self._callback_lock:
            self.key_callback = None

        # print(f"[DEBUG] 设备已关闭")

    # 断开连接清楚所有显示
    def disconnected(self):
        self.transport.disconnected()

    # 清除某个按键的图标
    def cleaerIcon(self, index):
        origin = index
        index = self.key(index)
        if index not in range(1, 16):
            print(f"key '{origin}' out of range. you should set (1 ~ 15)")
            return -1
        self.transport.keyClear(index)

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
        :argtypes:存放信息的字节数组, 字节数组的长度建议512

        """
        data = self.transport.read_(13)
        return data

    # 一直检测设备有无信息反馈，建议开线程使用
    def whileread(self):
        while 1:
            try:
                data = self.read()
                if data != None and len(data) >= 11:
                    # if data[9] == 0xFF:
                    #     print("写入成功")
                    if 0:
                        pass
                    else:
                        if data[:3].decode("utf-8", errors="ignore") == "ACK" and data[
                            5:7
                        ].decode("utf-8", errors="ignore"):
                            # print(data[0: 10])
                            if data[10] == 0x01 and data[9] > 0x00 and data[9] <= 0x0F:
                                if self.KEY_MAP:
                                    print(
                                        "按键{}".format(KEY_MAPPING[data[9]]) + "被按下"
                                    )
                                else:
                                    print("按键{}".format(data[9]) + "被按下")
                            elif (
                                data[10] == 0x00 and data[9] > 0x00 and data[9] <= 0x0F
                            ):
                                if self.KEY_MAP:
                                    print(
                                        "按键{}".format(KEY_MAPPING[data[9]]) + "抬起"
                                    )
                                else:
                                    print("按键{}".format(data[9]) + "抬起")
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
    # 设置定时器时间
    def set_seconds(self, data):
        self.__seconds = data
        self.reset_Countdown(self.__seconds)

    # 重启定时器
    def reset_Countdown(self, data):
        self.screenlicent.cancel()
        self.screenlicent = threading.Timer(data, self.screen_Off)
        self.screenlicent.start()

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
    def set_touchscreen_image(self, image):
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
                            # print("写入成功")
                            pass
                        else:
                            # k = KEY_MAPPING[arr[9]]
                            try:
                                k = self.key(arr[9])
                                new = arr[10]
                                if new == 0x02:
                                    new = 0
                                if new == 0x01:
                                    new = 1
                                
                                # CRITICAL: Thread-safe callback invocation with proper isolation
                                # Make copies of everything BEFORE acquiring lock to minimize lock time
                                key_copy = int(k) if isinstance(k, int) else k
                                state_copy = int(new)
                                
                                # Get callback reference with lock
                                with self._callback_lock:
                                    callback = self.key_callback
                                
                                # Call callback OUTSIDE of lock to avoid deadlocks
                                if callback is not None:
                                    try:
                                        # Import threading to ensure GIL is properly managed
                                        import threading
                                        # Call callback - Python threading will handle GIL
                                        callback(self, key_copy, state_copy)
                                    except Exception as callback_error:
                                        print(
                                            f"按键回调函数发生错误: {callback_error}",
                                            flush=True,
                                        )
                                        traceback.print_exc()
                            except Exception as key_error:
                                print(f"处理按键数据时发生错误: {key_error}", flush=True)
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
        changes state (either pressed, or released).

        .. note:: This callback will be fired from an internal reader thread.
                  Ensure that the given callback function is thread-safe.

        .. note:: Only one callback can be registered at one time.

        .. seealso:: See :func:`~StreamDock.set_key_callback_async` method for
                     a version compatible with Python 3 `asyncio` asynchronous
                     functions.

        :param function callback: Callback function to fire each time a button
                                state changes.
        """
        with self._callback_lock:
            self.key_callback = callback

    def set_key_callback_async(self, async_callback, loop=None):
        """
        Sets the asynchronous callback function called each time a button on the
        StreamDock changes state (either pressed, or released). The given
        callback should be compatible with Python 3's `asyncio` routines.

        .. note:: The asynchronous callback will be fired in a thread-safe
                  manner.

        .. note:: This will override the callback (if any) set by
                  :func:`~StreamDock.set_key_callback`.

        :param function async_callback: Asynchronous callback function to fire
                                        each time a button state changes.
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
