# StreamDock Python SDK

## Supported Platforms

| Platform           | Support Status | Description                                                 |
| ------------------ | -------------- | ----------------------------------------------------------- |
| Linux (x64, arm64) | ✅ Supported   | Ubuntu 20.04+ recommends using pyudev for device monitoring |
| Windows (x64)      | ✅ Supported   | Supports WMI and polling mode                               |
| macOS (x64, arm64) | ✅ Supported   | Uses polling mode for device monitoring                     |

## Installation Guide

### 🔧 Linux Platform

> [Recommended Environment] Ubuntu 20.04 + Python 3.10 or later

#### 1. Python Dependencies

```bash
pip install -r requirements.txt
```

#### 2. System Library Dependencies

```bash
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0
```

> ⚠️ **Important**:
>
> Must install `libusb-1.0-0-dev` before installing `libhidapi-libusb0`
>
> Requires sudo privileges to run

### 🔧 Windows Platform

> [Recommended Environment] Windows 10/11 + Python 3.10

#### 1. Python Dependencies

```bash
pip install -r requirements.txt
```

#### 2. Drivers

Windows 10/11 usually automatically installs the required drivers. If you encounter issues, please install:

- [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)
- [HIDAPI Windows Driver](https://github.com/libusb/hidapi)

### 🔧 macOS Platform

> [Recommended Environment] macOS 11 + Python 3.10 or later

#### 1. Python Dependencies

```bash
pip install -r requirements.txt
```

#### 2. System Dependencies

```bash
brew install hidapi
```

## Quick Start

The following is a complete usage example showing how to enumerate devices, set images, and listen for key events:

```python
from StreamDock.DeviceManager import DeviceManager
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro
import threading
import time


def key_callback(device, key, state):
    try:
        action = "pressed" if state == 1 else "released"
        print(f"Key {key} was {action}", flush=True)
    except Exception as e:
        print(f"Key callback error: {e}", flush=True)
        import traceback
        traceback.print_exc()


def main():
    # Wait a moment to ensure previous instances release resources
    time.sleep(0.5)

    manner = DeviceManager()
    streamdocks = manner.enumerate()

    if not streamdocks:
        print("No Stream Dock device found")
        return

    # Listen for device plug/unplug
    t = threading.Thread(target=manner.listen)
    t.daemon = True
    t.start()

    print("Found {} Stream Dock(s).\n".format(len(streamdocks)))

    for device in streamdocks:
        # Open device
        try:
            device.open()
            device.init()
        except Exception as e:
            print(f"Failed to open device: {e}")
            import traceback
            traceback.print_exc()
            raise
        print(f"path: {device.path}, firmware_version: {device.firmware_version} serial_number: {device.serial_number}")
        # Set background image
        res = device.set_touchscreen_image("img/backgroud_test.png")
        device.refresh()
        time.sleep(2)
        for i in range(1,2):
            device.set_key_image(i, "img/button_test.jpg")
            device.refresh()
        time.sleep(0.5)
        if isinstance(device, StreamDockN4Pro):
            device.set_led_brightness(100)
            device.set_led_color(0, 0, 255)
        device.set_key_callback(key_callback)

    print("Program is running, press Ctrl+C to exit...")
    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nShutting down devices...")
    except Exception as e:
        print(f"\nMain loop exception: {e}")
        import traceback
        traceback.print_exc()
    finally:
        # Ensure all devices are properly closed
        for device in streamdocks:
            try:
                device.close()
            except Exception as e:
                print(f"Error closing device: {e}")
        print("Program exited")

if __name__ == "__main__":
    try:
        main()
    except SystemExit as e:
        print(f"\nProgram terminated by SystemExit: {e}")
        import traceback
        traceback.print_exc()
    except Exception as e:
        print(f"\nUncaught exception: {e}")
        import traceback
        traceback.print_exc()
```

### Basic Steps Explanation

1. **Import necessary modules**

   ```python
   from StreamDock.DeviceManager import DeviceManager
   ```
2. **Enumerate devices**

   ```python
   manager = DeviceManager()
   devices = manager.enumerate()
   ```
3. **Open and initialize device**

   ```python
   device.open()
   device.init()
   ```
4. **Set images**

   ```python
   # Set touchscreen background
   device.set_touchscreen_image("path/to/image.png")

   # Set key icon
   device.set_key_image(key_index, "path/to/icon.jpg")

   # Refresh display
   device.refresh()
   ```
5. **LED Control (e.g. N4 Pro)**

   ```python
   if isinstance(device, StreamDockN4Pro):
       device.set_led_brightness(100)  # Set LED brightness (0-100)
       device.set_led_color(0, 0, 255)  # Set LED color (R, G, B)
   ```
6. **Listen for key events**

   ```python
   def key_callback(device, key, state):
       # state=1 means pressed, state=0 means released
       pass

   device.set_key_callback(key_callback)
   ```
7. **Clean up resources**

   ```python
   device.close()
   ```

## Important Notes

### 1. Thread Management

- Recommend using `threading.Thread(..., daemon=True)` to create background threads
- Device listening and feedback threads need to run continuously, recommend setting as daemon threads
- Use boolean variables to control loop start/stop

### 2. Platform Differences

Only newer platforms support **asynchronous response** to key events and device response events:

| Platform                           | Listen to keys while setting images | Notes                              |
| ---------------------------------- | ----------------------------------- | ---------------------------------- |
| 293V3 / N4 / N4Pro / XL / M18 / M3 | ✅ Supported                        | Multi-tasking                      |
| 293 / 293s                         | ❌ Not supported                    | Must wait for operation completion |

On older devices (like 293 and 293s), when calling `set_key_image` or `set_touchscreen_image`, the device cannot simultaneously respond to key operations.

### 3. Hotplug and Auto-Recovery

When a device is unplugged and reinserted, `DeviceManager.listen()` will automatically recognize and reconnect.

It is recommended to perform initialization after device reconnection:

```python
def autoInit(device):
    """Device auto-initialization function"""
    device.set_key_image(1, "img/default.png")
    device.refresh()
    # Other initialization operations...

# Use in reconnection callback
newDevice.open()
autoInit(newDevice)
```

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) file for details.

## Contributing

Welcome to submit Issues and Pull Requests to improve this project.
