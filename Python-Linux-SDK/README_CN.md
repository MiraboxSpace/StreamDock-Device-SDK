# Linux-x86_64 platform

### 🔧 Linux-x86_64 平台 SDK 配置指南

> 【推荐环境】Ubuntu 20.04 + Python 3.8.10 或更新版

---

### 🐍 Python 依赖

安装必要的 Python 套件：

```bash
pip install pillow pyudev
```

> 注：`threading` 、`ctypes`、`time`、`abc` 是 Python 标准库，无需安装

---

### 📦 系统库依赖

```bash
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0
```

> 重要：必须先安装 `libusb-1.0-0-dev` ，然后再安装 `libhidapi-libusb0`

如果出现错误：

```
undefined reference to `get_input_report()`
```

请把 `/usr/local/lib/libhidapi-libusb.so.0` 替换成 SDK 上提供的 Transport 文件夹中的 `libhidapi-libusb.so.0`

---

### 快速开始示例

```python
from StreamDock.DeviceManager import DeviceManager
from StreamDock.Devices.StreamDockN1 import StreamDockN1
import threading
import time

if __name__ == "__main__":
    manner = DeviceManager()
    streamdocks= manner.enumerate()
    # 监听设备插拔
    t = threading.Thread(target=manner.listen)
    t.daemon = True 
    t.start()
    print("Found {} Stream Dock(s).".format(len(streamdocks)))
    for device in streamdocks:
        # 打开设备
        device.open()
        device.init()
        # 开线程获取设备反馈
        t = threading.Thread(target = device.whileread)
        t.daemon = True
        t.start()
        #设置背景图片
        res = device.set_touchscreen_image("../img/YiFei320.png")
        device.refresh()
        time.sleep(2)
        for i in range(1, 19):
            device.set_key_image(i, "../img/tiga64.png")
            device.refresh()
        time.sleep(2)
        # 清空某个按键的图标
        device.cleaerIcon(3)
        device.refresh()
        time.sleep(1)
        # 清空所有按键的图标
        device.clearAllIcon()
        device.refresh()
        time.sleep(0)
        # N1 switch mode
        if isinstance(device, StreamDockN1):
            device.switch_mode(0)
        # 关闭设备
        # device.close()
    time.sleep(10000)
```

根据需求修改图片路径

---

### ⚠️ 重要说明

#### 1. 线程管理

推荐使用 `threading.Thread(..., daemon=True)` 创建被线程

如果想控制无限循环的启动/停止，可以通过 bool 变量来实现

#### 2. 旧新平台区别

只有新平台支持 **异步响应** 按键事件和设备响应事件

| 平台       | 设置图片同时支持按键监听 | 备注                         |
| ------------ | -------------------------- | ------------------------------ |
| 293V3 / N4 | ✅ 支持                  | 设置图片时可同时检测按键     |
| 293 / 293s | ❌ 不支持                | 必须等图片设置完成后才能监听 |

在老型设备上，如 293 和 293s，当调用 `setKeyImg` 或 `setTouchScreen`时，设备无法同时响应用户按键操作，必须等待调用结束，确保设备处于空闲状态后，才能进行键盘监听。

#### 3. 🔌 热插拔与自动恢复

当设备被拔出并重新插入时，`DeviceManager.listen()` 会自动识别新插入的设备并进行重连。

你可以在设备重新连接后执行个性化的初始化逻辑，例如设置按键图标、刷新状态：

```python
# your reconnect logic like the next two line
newDevice.set_key_image(1, "../img/tiga64.png")
newDevice.refresh()
```

建议将这些逻辑封装为一个初始化函数，例如 `autoInit()`，以便统一调用：

```python
newDevice.open()
newDevice.autoInit()  # 比如设置按键图标、刷新屏幕、切换模式等
```

确保设备在断开并重连后能自动恢复到之前的工作状态。

> macOS 平台通过轮询（`hid.enumerate()`）方式模拟监听；Linux 则使用 pyudev 提供的 USB 插拔事件。
