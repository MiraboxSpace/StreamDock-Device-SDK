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
>
> 需要使用 sudo 权限运行

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
    """按键事件回调函数"""
    action = "按下" if state == 1 else "释放"
    print(f"按键{key}被{action}", flush=True)

def main():
    # 创建设备管理器
    manager = DeviceManager()

    # 枚举所有连接的 Stream Dock 设备
    streamdocks = manager.enumerate()

    if not streamdocks:
        print("未找到 Stream Dock 设备")
        return

    # 启动设备监听线程（支持热插拔）
    listen_thread = threading.Thread(target=manager.listen)
    listen_thread.daemon = True
    listen_thread.start()

    print(f"找到 {len(streamdocks)} 个 Stream Dock 设备")

    # 遍历并初始化每个设备
    for device in streamdocks:
        try:
            # 打开并初始化设备
            device.open()
            device.init()

            # 显示设备信息
            print(f"设备路径: {device.path}")
            print(f"固件版本: {device.firmware_version}")
            print(f"序列号: {device.serial_number}")

            # 设置触摸屏背景图片
            device.set_touchscreen_image("img/background.png")

            # 设置按键图标
            for i in range(1, 5):  # 设置前4个按键的图标
                device.set_key_image(i, f"img/button_{i}.jpg")

            # 刷新屏幕显示
            device.refresh()

            # 注册按键事件回调
            device.set_key_callback(key_callback)

        except Exception as e:
            print(f"初始化设备失败: {e}")

    print("程序正在运行，按 Ctrl+C 退出...")

    try:
        # 主循环
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n正在关闭设备...")
    finally:
        # 清理资源
        for device in streamdocks:
            try:
                device.close()
            except Exception as e:
                print(f"关闭设备时出错: {e}")
        print("程序已退出")

if __name__ == "__main__":
    main()
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
5. **监听按键事件**

   ```python
   def key_callback(device, key, state):
       # state=1 表示按下，state=0 表示释放
       pass

   device.set_key_callback(key_callback)
   ```
6. **清理资源**

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

当设备被拔出并重新插入时，`DeviceManager.listen()` 会自动识别并进行重连。

建议在设备重连后执行初始化：

```python
def autoInit(device):
    """设备自动初始化函数"""
    device.set_key_image(1, "img/default.png")
    device.refresh()
    # 其他初始化操作...

# 在重连回调中使用
newDevice.open()
autoInit(newDevice)
```

## 许可证

本项目采用 MIT 许可证。详情请见 [LICENSE](LICENSE) 文件。

## 贡献

欢迎提交 Issue 和 Pull Request 来改进这个项目。
