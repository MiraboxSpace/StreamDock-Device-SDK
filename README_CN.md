# Linux-x86_64 platform
### 推荐使用 Python 3 或以上版本。测试环境：Ubuntu 20.04，Python 3.8.10
  
你需要安装 `pillow`, `pyudev`, `threading`, `ctypes`, `time`, `abc`
  
```bash
pip install pillow
```
```bash
pip  install  pyudev
``` 

另外，以下库可能已经包含在你的 Python 版本中，无需额外安装：
```bash
pip  install  threading
```
```bash
pip  install  ctypes
```
```bash
pip  install  time
```
```bash
pip  install  abc
```
  
### 安装 Linux 软件
  
如果你想逐个安装，请注意安装顺序：`libusb-1.0-0-dev` 需要在 `libhidapi-libusb0` 之前安装。如果遇到错误 "`undefined reference to get_input_report()`"，你可以用我们在 Transport 文件夹中提供的 `libhidapi-libusb.so.0` 文件替换 `/usr/local/lib/libhidapi-libusb.so.0` 文件。
```
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0
```

使用时，你需要首先定义一个 `DeviceManager` 类对象（设备管理器类），然后调用其 `enumerate()` 函数遍历设备并获取设备对象列表。
```py
manner = DeviceManager();
streamdocks = manner.enumerate();
```
  
在获取设备对象列表后，需要调用 `open()` 方法打开设备，然后才能调用其他方法对设备进行操作。
  
~~**注意！使用 set_touchscreen_image() 和 set_key_image() 时，图像格式必须是 JPEG。**~~ 但是，在新版本中，你不需要担心图像大小，SDK 会自动为你处理这些细节。~~**
**另外，set_touchscreen_image() 函数要求固定图像大小。例如，293V3 的图像大小为 800x480。set_key_image() 函数也接受固定大小，例如 293V3 的大小为 112x112。具体尺寸，请参阅[文档](https://creator.key123.vip/en/windows/websocket/events-sent.html#setkeyimg)and[文档](https://creator.key123.vip/en/windows/websocket/events-sent.html#setbackgroundimg)~~
  
```py
from StreamDock.DeviceManager import DeviceManager
from StreamDock.ImageHelpers import PILHelper
import threading
import time

if __name__ == "__main__":
    manner = DeviceManager()
    streamdocks = manner.enumerate()
    # 监控设备插拔事件
    t = threading.Thread(target=manner.listen)
    t.daemon = True # 分离线程
    t.start()
    print("找到 {} 个 Stream Dock 设备。\n".format(len(streamdocks)))
    for device in streamdocks:
        # 打开设备
        device.open()
        device.wakeScreen()
        # 创建一个线程监听设备内容
        t = threading.Thread(target = device.whileread)
        t.daemon = True # 分离线程
        t.start()
        # 设置亮度 0-100
        device.set_brightness(50)
        # 设置背景图片
        res = device.set_touchscreen_image("../img/YiFei320.png")
        device.refresh()
        time.sleep(2)
        for i in range(1, 19):
            device.set_key_image(i, "../img/tiga64.png")
            device.refresh()
        time.sleep(2)
        # 清除某个按键的图标 [1, x]
        device.cleaerIcon(3)
        device.refresh()
        time.sleep(1)
        # 清除所有按键图标
        device.clearAllIcon()
        time.sleep(1)
        # 关闭设备
        # device.close()
    time.sleep(10000)
```
### 注意事项!
我们建议你使用我们的 SDK 来监听设备的插拔事件或按键按下事件。这样做时，尽量使用 `threading.Thread.daemon = True` 来正确控制子线程的生命周期，确保主线程的生命周期要长于子线程的生命周期（因为监听设备事件和按键事件几乎是死循环的操作）。如果你想修改死循环的逻辑并控制这两个监听事件的启动和停止，可以在这两个函数里面增加控制启动或停止的布尔变量。

另一个需要注意的重要点是，只有较新的平台才支持异步检测按键按下事件和其他设备事件（设备响应）。例如，在** 293V3 和 N4 **平台上，你可以在设置 `setKeyImg` 或 `setTouchScreen` 时同时检测按键按下事件。然而，在旧平台上，如 **293 和 293s**，不支持异步设备响应。在这些旧平台上，你不能在执行 `setKeyImg` 或 `setTouchScreen` 时检测按键按下事件；必须等到 `setKeyImg` 或 `setTouchScreen` 事件完成后，设备处于空闲状态时才能使用按键监听。这就是新旧平台之间的差异。
