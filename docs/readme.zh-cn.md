```markdown
<p align="center">  
  <img src="_Images/logo.png" height="128">  
  <h1 align="center">StreamDock 设备 SDK</h1>  
</p>  
  
<p align="center">  
  <a aria-label="加入 Discord 社区" href="https://discord.gg/WvCkKRGavX">  
    <img alt="" src="https://img.shields.io/badge/加入%20Discord-5865F2.svg?style=for-the-badge&logo=Discord&logoColor=white">  
  </a>  
</p>  
  
## 目录  
  
- [概述](#概述)  
- [支持的设备](#支持的设备)  
- [Python SDK](#python-sdk)  
  - [支持的平台](#支持的平台-python)  
  - [安装](#安装-python)  
  - [快速开始](#快速开始-python)  
  - [Python API 对象](#python-api-对象)  
- [C++ SDK](#c-sdk)  
  - [Linux 安装](#linux-安装-c)  
  - [Windows 安装](#windows-安装-c)  
  - [macOS 安装](#macos-安装-c)  
  - [C++ API 对象](#c-api-对象)  
- [WebSocket SDK](#websocket-sdk)  
  - [安装](#安装-websocket)  
  - [WebSocket API 事件](#websocket-api-事件)  
- [故障排除](#故障排除)  
- [许可证](#许可证)  
  
---  
  
## 概述  
  
**StreamDock 设备 SDK** 使您能够通过代码直接控制 StreamDock 设备。该项目提供三个独立的 SDK：  
  
| SDK | 语言 | 平台 |  
|-----|------|------|  
| Python SDK | Python 3.10+ | Linux、Windows、macOS |  
| C++ SDK | C++17 | Linux、Windows、macOS |  
| WebSocket SDK | 任何（WebSocket 协议） | Windows |  
  
完整官方文档：[https://creator.key123.vip/en/guide/get-started.html](https://creator.key123.vip/en/guide/get-started.html)  
  
---  
  
## 支持的设备  
  
| 型号 | 系列 |  
|------|------|  
| StreamDock 293 | 293、293s、293V3、293sV3 |  
| StreamDock N | N1、N3、N4、N4 Pro |  
| StreamDock XL | XL |  
| StreamDock M | M3、M18 |  
| K1 Pro | K1 Pro |  
  
---  
  
## Python SDK  
  
### 支持的平台 (Python)  
  
| 平台 | 状态 | 说明 |  
|------|------|------|  
| Linux (x64、arm64) | ✅ 支持 | Ubuntu 20.04+，建议使用 pyudev 进行监控 |  
| Windows (x64) | ✅ 支持 | 支持 WMI 和轮询模式 |  
| macOS (x64、arm64) | ✅ 支持 | 使用轮询模式进行监控 |  
  
### 安装 (Python)  
  
#### Linux  
  
> 推荐环境：Ubuntu 20.04 + Python 3.10 或更高版本  
  
**1. Python 依赖：**  
  
```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

**2. 系统依赖：**  
```bash  
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0  
```  
⚠️ **重要：** `libusb-1.0-0-dev` 包必须在 `libhidapi-libusb0` 之前安装。需要使用 `sudo`。  

#### Windows  
  
> 推荐环境：Windows 10/11 + Python 3.10  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

驱动程序通常会自动安装。如果出现问题，请安装：  
- [Microsoft Visual C++ 可再发行程序包](https://aka.ms/vs/17/release/vc_redist.x64.exe)  
- [HIDAPI Windows 驱动程序](https://github.com/libusb/hidapi)  

#### macOS  
  
> 推荐环境：macOS 11 + Python 3.10 或更高版本  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
brew install hidapi  
```  

### 快速开始 (Python)  

```python
from StreamDock.DeviceManager import DeviceManager  
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro  
import threading  
import time  

def key_callback(device, event):  
    from StreamDock.InputTypes import EventType  
    if event.event_type == EventType.BUTTON:  
        action = "按下" if event.state == 1 else "释放"  
        print(f"按键 {event.key.value} 已{action}", flush=True)  

def main():  
    time.sleep(0.5)  
    manager = DeviceManager()  
    devices = manager.enumerate()  

    if not devices:  
        print("未找到 StreamDock 设备")  
        return  

    # 监听连接/断开连接事件  
    t = threading.Thread(target=manager.listen)  
    t.daemon = True  
    t.start()  

    print(f"找到 {len(devices)} 个设备。\n")  

    for device in devices:  
        device.open()  
        device.init()  
        print(f"路径：{device.path}，固件：{device.firmware_version}，序列号：{device.serial_number}")  

        # 设置背景图像  
        device.set_touchscreen_image("img/backgroud_test.png")  
        device.refresh()  
        time.sleep(2)  

        # 设置按键图像  
        device.set_key_image(1, "img/button_test.jpg")  
        device.refresh()  

        # LED 控制（仅限 N4 Pro）  
        if isinstance(device, StreamDockN4Pro):  
            device.set_led_brightness(100)  
            device.set_led_color(0, 0, 255)  

        # 注册事件回调  
        device.set_key_callback(key_callback)  

    print("程序正在运行，按 Ctrl+C 退出...")  
    try:  
        while True:  
            time.sleep(0.1)  
    except KeyboardInterrupt:  
        print("\n关闭设备...")  
    finally:  
        for device in devices:  
            device.close()  
        print("程序已终止")  

if __name__ == "__main__":  
    main()  
```

### Python API 对象  

**DeviceManager** (`Python-SDK/src/StreamDock/DeviceManager.py`)  
中央设备管理器。负责枚举、连接和热插拔监控。  

| 方法 | 说明 |  
|------|------|  
| `enumerate() -> list` | 枚举所有连接的 StreamDock 设备。返回 `StreamDock` 对象列表。 |  
| `listen()` | 启动热插拔监控（连接/断开连接）。必须在单独的线程中运行。 |  

**StreamDock** (基础抽象类) (`Python-SDK/src/StreamDock/Devices/StreamDock.py`)  
所有设备的基础类。不应直接实例化。  

| 方法 | 说明 |  
|------|------|  
| `open()` | 打开设备连接。启动读取线程和心跳。 |  
| `init()` | 初始化设备：唤醒屏幕、设置亮度、清除图标、获取固件。 |  
| `close()` | 关闭设备并释放所有资源。 |  
| `set_key_image(key, path)` | 为特定按键设置图像（索引从 1 开始）。 |  
| `set_touchscreen_image(path)` | 设置触摸屏背景图像。 |  
| `set_brightness(percent)` | 设置屏幕亮度 (0–100)。 |  
| `refresh()` | 更新设备显示。 |  

**特定设备类**  
每个型号都继承自 `StreamDock` 并实现抽象方法：  

| 类 | 文件 | 型号 |  
|----|------|------|  
| `StreamDock293` | `Devices/StreamDock293.py` | 293 |  
| `StreamDock293V3` | `Devices/StreamDock293V3.py` | 293V3 |  
| `StreamDockN4Pro` | `Devices/StreamDockN4Pro.py` | N4 Pro |  
| `K1Pro` | `Devices/K1Pro.py` | K1 Pro |  

---  

## C++ SDK  
  
### Linux 安装 (C++)  

> 推荐环境：Ubuntu 20.04+、CMake 3.16+、C++17  

1. **系统依赖：**  
```bash  
sudo apt update  
sudo apt install build-essential cmake  
sudo apt install -y libudev-dev libusb-1.0-0-dev libgif-dev  
```  

2. **OpenCV：**  
```bash  
sudo apt install libopencv-dev  
```  

3. **编译：**  
```bash  
cd CPP-SDK  
chmod +x build.sh  
./build.sh  
```  

4. **运行：**  
```bash  
sudo ./bin/main  
```  

**避免使用 `sudo`：** 将 `99-streamdock.rules` 复制到 `/etc/udev/rules.d/` 并运行：  
```bash  
sudo udevadm control --reload-rules  
sudo udevadm trigger  
```  

### Windows 安装 (C++)  

> 推荐环境：Visual Studio 2022、C++17  

`bin/` 目录中的预编译库：  
- `opencv_core4120.dll`  
- `opencv_imgcodecs4120.dll`  
- `opencv_imgproc4120.dll`  
- `hidapi.dll`  

**编译：**  
```bash  
cd CPP-SDK  
build.bat  
```  

或在 Visual Studio 2022 中打开并使用 `Ctrl+B` 编译。  

### macOS 安装 (C++)  

1. **准备 OpenCV：**  
```bash  
cd CPP-SDK/ImgProcesser/third_party  
bash link_opencv_symlinks.sh  
```  

2. **编译：**  
```bash  
cd CPP-SDK  
chmod +x build.sh  
./build.sh  
```  

---  

## WebSocket SDK  

### 安装 (WebSocket)  

运行 `WebsocketSDK.exe`（默认端口：9002）：  
```bash  
WebsocketSDK.exe  
# 或使用自定义端口：  
WebsocketSDK.exe -port 9009  
```  

通过 WebSocket 连接：`ws://127.0.0.1:9002`  

**Python 测试依赖：**  
```bash  
pip install websockets  
```  

### WebSocket API 事件  

所有消息都使用 JSON 格式：  
```json  
{  
  "event": "事件名称",  
  "path": "设备路径_Base64编码",  
  "payload": {}  
}  
```  

**主要客户端命令：**  
| 事件 | 负载 | 说明 |  
|------|------|------|  
| `setBrightness` | `{ "brightness": 0-100 }` | 设置屏幕亮度 |  
| `setKeyImg` | `{ "keyIndex": N, "imagePath": "路径" }` | 设置按键图标 |  
| `refresh` | `{}` | 更新屏幕 |  
```
