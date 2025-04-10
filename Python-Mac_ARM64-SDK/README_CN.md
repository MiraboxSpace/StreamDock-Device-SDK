# StreamDock SDK for Mac（中文）

### 🔧 Mac-Arm64 平台 SDK 配置指南

> ✅ 推荐环境：Python 3.8.10 或以上版本

---


### 📦 系统库依赖

使用 SDK 前，请先安装系统级库：

```bash
sudo brew install hidapi
```

---

### 🐍 Python 依赖

安装必要的 Python 库：
如果你安装了 Homebrew，必须在 pip 命令中添加 `--break-system-packages` 参数：

```bash
python3 -m pip install Pillow --break-system-packages
python3 -m pip install hid --break-system-packages
```

> `threading`、`ctypes`、`time` 和 `abc` 是 Python 标准库的一部分，**无需安装**

---

### 🚀 快速开始示例

```python
from StreamDock.DeviceManager import DeviceManager
from StreamDock.Devices.StreamDockN1 import StreamDockN1
import threading
import time

if __name__ == "__main__":
    manner = DeviceManager()
    streamdocks= manner.enumerate()
    # 启动设备监听（轮询方式）
    manner.listen()
    print("Found {} Stream Dock(s).".format(len(streamdocks)))
    for device in streamdocks:
        # 打开设备
        device.open()
        device.init()
        # 开新线程获取设备反馈
        t = threading.Thread(target = device.whileread)
        t.daemon = True
        t.start()
        # 设置背景图片
        res = device.set_touchscreen_image("../img/YiFei320.jpg")
        device.refresh()
        time.sleep(2)
        for i in range(1, 19):
            device.set_key_image(i, "../img/tiga64.jpg")
            device.refresh()
        time.sleep(2)
        # 清除某个按键图标
        device.cleaerIcon(3)
        device.refresh()
        time.sleep(1)
        # 清除所有按键图标
        device.clearAllIcon()
        device.refresh()
        time.sleep(0)
        # 切换模式（N1）
        if isinstance(device, StreamDockN1):
            device.switch_mode(0)
        # 关闭设备
        # device.close()
    time.sleep(10000)
```

---

### ⚠️ 重要说明

#### 1. 线程管理

使用 `threading.Thread(..., daemon=True)` 启动监听和读取线程。

这些线程是常驻线程，如需手动控制线程退出，可以使用布尔变量控制循环退出。

#### 2. 新旧平台区别

仅新型号支持在设置图片过程中仍能接收按键事件：

| 平台       | 设置图片时支持按键监听 | 说明                            |
|------------|------------------------|---------------------------------|
| 293V3 / N4 | ✅ 支持                | 可在 setKeyImg/setTouchScreen 期间响应按键 |
| 293 / 293s | ❌ 不支持              | 必须等待图片设置结束再监听按键 |

旧设备（如 293, 293s）设置图片期间无法监听按键，需等待操作结束后设备空闲再监听。

#### 3. 🔌 热插拔与自动恢复

当 USB 设备重新插入时，SDK 会自动重新连接，并支持执行初始化操作。

你可以在重连后设置自定义初始化逻辑，例如：

```python
# your reconnect logic like the next two line
newDevice.set_key_image(1, "../img/tiga64.jpg")
newDevice.refresh()
```

推荐封装为 `autoInit()` 方法调用：

```python
newDevice.open()
newDevice.autoInit()  # 比如设置图标、刷新界面、切换模式等
```

> macOS 使用 `hid.enumerate()` 每秒轮询一次以模拟插拔事件。
