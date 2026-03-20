```markdown
<p align="center">  
  <img src="_Images/logo.png" height="128">  
  <h1 align="center">StreamDock Device SDK</h1>  
</p>  
  
<p align="center">  
  <a aria-label="Join Discord Community" href="https://discord.gg/WvCkKRGavX">  
    <img alt="" src="https://img.shields.io/badge/Join%20Discord-5865F2.svg?style=for-the-badge&logo=Discord&logoColor=white">  
  </a>  
</p>  
  
## Table of Contents  
  
- [Overview](#overview)  
- [Supported Devices](#supported-devices)  
- [Python SDK](#python-sdk)  
  - [Supported Platforms](#supported-platforms-python)  
  - [Installation](#installation-python)  
  - [Quick Start](#quick-start-python)  
  - [Python API Objects](#python-api-objects)  
- [C++ SDK](#c-sdk)  
  - [Linux Installation](#installation-linux-c)  
  - [Windows Installation](#installation-windows-c)  
  - [macOS Installation](#installation-macos-c)  
  - [C++ API Objects](#cpp-api-objects)  
- [WebSocket SDK](#websocket-sdk)  
  - [Installation](#installation-websocket)  
  - [WebSocket API Events](#websocket-api-events)  
- [Troubleshooting](#troubleshooting)  
- [License](#license)  
  
---  
  
## Overview  
  
The **StreamDock Device SDK** enables direct control of StreamDock devices through code. The project offers three independent SDKs:  
  
| SDK | Language | Platforms |  
|-----|----------|-----------|  
| Python SDK | Python 3.10+ | Linux, Windows, macOS |  
| C++ SDK | C++17 | Linux, Windows, macOS |  
| WebSocket SDK | Any (WebSocket protocol) | Windows |  
  
Full official documentation: [https://creator.key123.vip/en/guide/get-started.html](https://creator.key123.vip/en/guide/get-started.html)  
  
---  
  
## Supported Devices  
  
| Model | Series |  
|-------|--------|  
| StreamDock 293 | 293, 293s, 293V3, 293sV3 |  
| StreamDock N | N1, N3, N4, N4 Pro |  
| StreamDock XL | XL |  
| StreamDock M | M3, M18 |  
| K1 Pro | K1 Pro |  
  
---  
  
## Python SDK  
  
### Supported Platforms (Python)  
  
| Platform | Status | Description |  
|----------|--------|-------------|  
| Linux (x64, arm64) | ✅ Supported | Ubuntu 20.04+, pyudev recommended for monitoring |  
| Windows (x64) | ✅ Supported | Supports WMI and polling mode |  
| macOS (x64, arm64) | ✅ Supported | Uses polling mode for monitoring |  
  
### Installation (Python)  
  
#### Linux  
  
> Recommended environment: Ubuntu 20.04 + Python 3.10 or higher  
  
**1. Python dependencies:**  
  
```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

**2. System dependencies:**  
```bash  
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0  
```  
⚠️ **Important:** The `libusb-1.0-0-dev` package must be installed before `libhidapi-libusb0`. Requires `sudo`.  

#### Windows  
  
> Recommended environment: Windows 10/11 + Python 3.10  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

Drivers are usually installed automatically. If issues occur, install:  
- [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)  
- [HIDAPI Windows Driver](https://github.com/libusb/hidapi)  

#### macOS  
  
> Recommended environment: macOS 11 + Python 3.10 or higher  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
brew install hidapi  
```  

### Quick Start (Python)  

```python
from StreamDock.DeviceManager import DeviceManager  
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro  
import threading  
import time  

def key_callback(device, event):  
    from StreamDock.InputTypes import EventType  
    if event.event_type == EventType.BUTTON:  
        action = "pressed" if event.state == 1 else "released"  
        print(f"Key {event.key.value} was {action}", flush=True)  

def main():  
    time.sleep(0.5)  
    manager = DeviceManager()  
    devices = manager.enumerate()  

    if not devices:  
        print("No StreamDock devices found")  
        return  

    # Listen for connect/disconnect events  
    t = threading.Thread(target=manager.listen)  
    t.daemon = True  
    t.start()  

    print(f"Found {len(devices)} device(s).\n")  

    for device in devices:  
        device.open()  
        device.init()  
        print(f"path: {device.path}, firmware: {device.firmware_version}, serial: {device.serial_number}")  

        # Set background image  
        device.set_touchscreen_image("img/backgroud_test.png")  
        device.refresh()  
        time.sleep(2)  

        # Set key image  
        device.set_key_image(1, "img/button_test.jpg")  
        device.refresh()  

        # LED control (N4 Pro only)  
        if isinstance(device, StreamDockN4Pro):  
            device.set_led_brightness(100)  
            device.set_led_color(0, 0, 255)  

        # Register event callback  
        device.set_key_callback(key_callback)  

    print("Program running, press Ctrl+C to exit...")  
    try:  
        while True:  
            time.sleep(0.1)  
    except KeyboardInterrupt:  
        print("\nClosing devices...")  
    finally:  
        for device in devices:  
            device.close()  
        print("Program terminated")  

if __name__ == "__main__":  
    main()  
```

### Python API Objects  

**DeviceManager** (`Python-SDK/src/StreamDock/DeviceManager.py`)  
Central device manager. Handles enumeration, connection, and hot-plug monitoring.  

| Method | Description |  
|--------|-------------|  
| `enumerate() -> list` | Enumerates all connected StreamDock devices. Returns list of `StreamDock` objects. |  
| `listen()` | Starts hot-plug monitoring (connect/disconnect). Must run in separate thread. |  

**StreamDock** (base abstract class) (`Python-SDK/src/StreamDock/Devices/StreamDock.py`)  
Base class for all devices. Should not be instantiated directly.  

| Method | Description |  
|--------|-------------|  
| `open()` | Opens device connection. Starts read thread and heartbeat. |  
| `init()` | Initializes device: wakes screen, sets brightness, clears icons, gets firmware. |  
| `close()` | Closes device and releases all resources. |  
| `set_key_image(key, path)` | Sets image for specific key (index starts at 1). |  
| `set_touchscreen_image(path)` | Sets touchscreen background image. |  
| `set_brightness(percent)` | Sets screen brightness (0–100). |  
| `refresh()` | Updates device display. |  

**Specific Device Classes**  
Each model inherits from `StreamDock` and implements abstract methods:  

| Class | File | Model |  
|-------|------|-------|  
| `StreamDock293` | `Devices/StreamDock293.py` | 293 |  
| `StreamDock293V3` | `Devices/StreamDock293V3.py` | 293V3 |  
| `StreamDockN4Pro` | `Devices/StreamDockN4Pro.py` | N4 Pro |  
| `K1Pro` | `Devices/K1Pro.py` | K1 Pro |  

---  

## C++ SDK  
  
### Linux Installation (C++)  

> Recommended environment: Ubuntu 20.04+, CMake 3.16+, C++17  

1. **System dependencies:**  
```bash  
sudo apt update  
sudo apt install build-essential cmake  
sudo apt install -y libudev-dev libusb-1.0-0-dev libgif-dev  
```  

2. **OpenCV:**  
```bash  
sudo apt install libopencv-dev  
```  

3. **Build:**  
```bash  
cd CPP-SDK  
chmod +x build.sh  
./build.sh  
```  

4. **Run:**  
```bash  
sudo ./bin/main  
```  

**To avoid `sudo`:** Copy `99-streamdock.rules` to `/etc/udev/rules.d/` and run:  
```bash  
sudo udevadm control --reload-rules  
sudo udevadm trigger  
```  

### Windows Installation (C++)  

> Recommended environment: Visual Studio 2022, C++17  

Precompiled libraries in `bin/`:  
- `opencv_core4120.dll`  
- `opencv_imgcodecs4120.dll`  
- `opencv_imgproc4120.dll`  
- `hidapi.dll`  

**Build:**  
```bash  
cd CPP-SDK  
build.bat  
```  

Or open in Visual Studio 2022 and build with `Ctrl+B`.  

### macOS Installation (C++)  

1. **Prepare OpenCV:**  
```bash  
cd CPP-SDK/ImgProcesser/third_party  
bash link_opencv_symlinks.sh  
```  

2. **Build:**  
```bash  
cd CPP-SDK  
chmod +x build.sh  
./build.sh  
```  

---  

## WebSocket SDK  

### Installation (WebSocket)  

Run `WebsocketSDK.exe` (default port: 9002):  
```bash  
WebsocketSDK.exe  
# or with custom port:  
WebsocketSDK.exe -port 9009  
```  

Connect via WebSocket: `ws://127.0.0.1:9002`  

**Python test dependencies:**  
```bash  
pip install websockets  
```  

### WebSocket API Events  

All messages use JSON format:  
```json  
{  
  "event": "event_name",  
  "path": "device_path_in_Base64",  
  "payload": {}  
}  
```  

**Main client commands:**  
| Event | Payload | Description |  
|-------|---------|-------------|  
| `setBrightness` | `{ "brightness": 0-100 }` | Set screen brightness |  
| `setKeyImg` | `{ "keyIndex": N, "imagePath": "path" }` | Set key icon |  
| `refresh` | `{}` | Update screen |  
```
