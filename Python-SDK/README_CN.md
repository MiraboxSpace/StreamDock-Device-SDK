# StreamDock Python SDK

## 支持的平台

| 平台               | 支持状态 | 说明                                   |
| ------------------ | -------- | -------------------------------------- |
| Linux (x64, arm64) | ✅ 支持  | Ubuntu 20.04+ 推荐使用 pyudev 监听设备 |
| Windows (x64)      | ✅ 支持  | 支持 WMI 和轮询模式                    |
| macOS (x64,arm64)  | ✅ 支持  | 使用轮询模式监听设备                   |

## 安装指南

### 🔧 Linux 平台

> 【推荐环境】Ubuntu 20.04 + Python 3.10 或更新版

#### 1. Python 依赖

```bash
pip install -r requirements.txt
```

#### 2. 系统库依赖

```bash
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0
```

> ⚠️ **重要**：
>
> 必须先安装 `libusb-1.0-0-dev`，然后再安装 `libhidapi-libusb0`

#### 3. 权限问题

有些 `Linux` 系统在没有添加用户设备权限的情况下,需要使用 `sudo` 权限运行,例如:

```
sudo python3 src/main.py
```

**添加设备权限列表方法:**

把 99-streamdock.rules 复制到  /etc/udev/rules.d/ 目录.后执行:

```
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### 🔧 Windows 平台

> 【推荐环境】Windows 10/11 + Python 3.10

#### 1. Python 依赖

```bash
pip install -r requirements.txt
```

#### 2. 驱动程序

Windows 10/11 通常会自动安装所需驱动。如遇到问题，请安装：

- [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)
- [HIDAPI Windows 驱动](https://github.com/libusb/hidapi)

### 🔧 macOS 平台

> 【推荐环境】macOS 11 + Python 3.10 或更新版

#### 1. Python 依赖

```bash
pip install -r requirements.txt
```

#### 2. 系统依赖

```bash
brew install hidapi
```

## 快速开始

以下是一个完整的使用示例，展示如何枚举设备、设置图像和监听按键事件：

```python
from StreamDock.DeviceManager import DeviceManager
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro
import threading
import time


def key_callback(device, key, state):
    try:
        action = "按下" if state == 1 else "释放"
        print(f"按键{key}被{action}", flush=True)
    except Exception as e:
        print(f"按键回调错误: {e}", flush=True)
        import traceback
        traceback.print_exc()


def main():
    # 等待一下，确保之前的实例释放了资源
    time.sleep(0.5)

    manner = DeviceManager()
    streamdocks = manner.enumerate()

    if not streamdocks:
        print("未找到 Stream Dock 设备")
        return

    # 监听设备插拔
    t = threading.Thread(target=manner.listen)
    t.daemon = True
    t.start()

    print("Found {} Stream Dock(s).\n".format(len(streamdocks)))

    for device in streamdocks:
        # 打开设备
        try:
            device.open()
            device.init()
        except Exception as e:
            print(f"打开设备失败: {e}")
            import traceback
            traceback.print_exc()
            raise
        print(f"path: {device.path}, firmware_version: {device.firmware_version} serial_number: {device.serial_number}")
        # 设置背景图片
        res = device.set_touchscreen_image("img/backgroud_test.png")
        device.refresh()
        time.sleep(2)
        for i in range(1,2):
            device.set_key_image(i, "img/button_test.jpg")
            device.refresh()
        time.sleep(0.5)
        if isinstance(device, StreamDockN4Pro):
            device.set_led_brightness(255)
            device.set_led_color(0, 0, 255)
        device.set_key_callback(key_callback)

    print("程序正在运行,按 Ctrl+C 退出...")
    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n正在关闭设备...")
    except Exception as e:
        print(f"\n主循环异常: {e}")
        import traceback
        traceback.print_exc()
    finally:
        # 确保所有设备都被正确关闭
        for device in streamdocks:
            try:
                device.close()
            except Exception as e:
                print(f"关闭设备时出错: {e}")
        print("程序已退出")

if __name__ == "__main__":
    try:
        main()
    except SystemExit as e:
        print(f"\n程序被SystemExit终止: {e}")
        import traceback
        traceback.print_exc()
    except Exception as e:
        print(f"\n未捕获的异常: {e}")
        import traceback
        traceback.print_exc()
```

### 基本步骤说明

1. **导入必要模块**

   ```python
   from StreamDock.DeviceManager import DeviceManager
   ```
2. **枚举设备**

   ```python
   manager = DeviceManager()
   devices = manager.enumerate()
   ```
3. **打开和初始化设备**

   ```python
   device.open()
   device.init()
   ```
4. **设置图像**

   ```python
   # 设置触摸屏背景
   device.set_touchscreen_image("path/to/image.png")

   # 设置按键图标
   device.set_key_image(key_index, "path/to/icon.jpg")

   # 刷新显示
   device.refresh()
   ```

   N4Pro、M3、XL 的按键图标支持传入 PNG 和 JPEG 文件。透明图片在必须转换为 JPEG 时，会先合成到纯黑背景上。
5. **LED 控制 (例如 N4 Pro)**

   ```python
   if isinstance(device, StreamDockN4Pro):
       device.set_led_brightness(100)  # 设置 LED 亮度 (0-100)
       device.set_led_color(0, 0, 255)  # 设置 LED 颜色 (R, G, B)
   ```
6. **监听按键事件**

   ```python
   def key_callback(device, key, state):
       # state=1 表示按下，state=0 表示释放
       pass

   device.set_key_callback(key_callback)
   ```
7. **清理资源**

   ```python
   device.close()
   ```

## 重要说明

### 1. 线程管理

- 推荐使用 `threading.Thread(..., daemon=True)` 创建后台线程
- 设备监听和反馈线程需要持续运行，建议设置为守护线程
- 使用布尔变量控制循环的启动/停止

### 2. 平台差异

只有新平台支持 **异步响应** 按键事件和设备响应事件：

| 平台                               | 设置图片时监听按键 | 备注             |
| ---------------------------------- | ------------------ | ---------------- |
| 293V3 / N4 / N4Pro / XL / M18 / M3 | ✅ 支持            | 多任务处理       |
| 293 / 293s                         | ❌ 不支持          | 必须等待操作完成 |

在老型设备上（如 293 和 293s），调用 `set_key_image` 或 `set_touchscreen_image` 时，设备无法同时响应按键操作。

### 3. 热插拔与自动恢复

当设备被拔出并重新插入时，`DeviceManager.listen()` 会自动识别设备变化：

- 插入设备：自动加入 `manager.streamdocks`
- 拔出设备：自动从 `manager.streamdocks` 移除，并调用 `close()` 释放资源
- 可通过 `on_device_added` / `on_device_removed` 处理业务恢复逻辑

建议把初始化逻辑封装成函数，并在设备重连回调中复用：

```python
def auto_init(device):
    """设备自动初始化函数"""
    device.open()
    device.init()
    device.set_key_image(1, "img/default.png")
    device.refresh()
    device.set_key_callback(key_callback)
    # 其他初始化操作...

t = threading.Thread(
    target=manager.listen,
    kwargs={
        "on_device_added": auto_init,
        "on_device_removed": lambda device: print(f"设备已移除: {device.path}"),
        "auto_open": False,
    },
)
t.daemon = True
t.start()
```

## 许可证

本项目采用 MIT 许可证。详情请见 [LICENSE](LICENSE) 文件。

## 贡献

欢迎提交 Issue 和 Pull Request 来改进这个项目。
