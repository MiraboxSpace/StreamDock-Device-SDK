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

#### 3. Permission Issues

On some `Linux` systems, if user device permissions have not been added, you need to run with `sudo` privileges, for example:

```bash
sudo python3 src/main.py
```

**How to add the device permission list:**

Copy `99-streamdock.rules` to the `/etc/udev/rules.d/` directory, then run:

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

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

   # Set animated GIF key icons. Use image_keys() to skip knobs or inputs
   # that do not have a key display.
   for key_index in device.image_keys():
       device.set_key_gif(key_index, "path/to/icon.gif")
   device.start_gif_loop()

   # Refresh display
   device.refresh()
   ```

   N4Pro, M3, and XL key icons support PNG and JPEG input files. When transparent images must be converted to JPEG for a device, transparent pixels are composited over black.

   N4Pro, XL, and M3 also support animated GIF backgrounds through frame background streaming:

   ```python
   # Background GIF is sent as temporary frame background data, not written to ROM.
   device.set_background_gif("path/to/background.gif")
   device.start_gif_loop()

   # Optional placement and framebuffer layer.
   device.set_background_gif("path/to/background.gif", x=0, y=0, fb_layer=0x00)

   # Stop GIF playback and clear the streamed background layer.
   device.stop_gif_loop()
   device.clear_background_gif()
   ```

   Background GIF is only supported on N4Pro, XL, and M3. On other devices, `set_background_gif()` returns `-1`.

   MP4 backgrounds use the same temporary frame background stream. Install the optional video dependency before using MP4 playback:

   ```bash
   pip install opencv-python
   ```

   ```python
   # Background MP4 is decoded frame by frame and looped.
   device.set_background_mp4("path/to/background.mp4")
   device.start_animation_loop()

   # Optional playback FPS override.
   device.set_background_mp4("path/to/background.mp4", fps=24)

   # Stop animation playback and clear the streamed background layer.
   device.stop_animation_loop()
   device.clear_background_animation()
   ```

   Background MP4 is only supported on N4Pro, XL, and M3. On other devices, `set_background_mp4()` returns `-1`. MP4 audio is ignored.
5. **LED Control (e.g. N4 Pro)**

   ```python
   if isinstance(device, StreamDockN4Pro):
       device.set_led_brightness(100)  # Set LED brightness (0-100)
       device.set_led_color(0, 0, 255)  # Set LED color (R, G, B)
   ```
6. **Device Config (N4Pro and XL)**

   N4Pro and XL expose a `device.config` object. Set the supported fields, then call `device.send_config()` to apply them.

   ```python
   from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro
   from StreamDock.Devices.StreamDockXL import StreamDockXL

   if isinstance(device, StreamDockN4Pro):
       device.config.enable_vibration = False
       device.config.led_follow_key_light = True
       device.send_config()

   if isinstance(device, StreamDockXL):
       device.config.led_follow_key_light = True
       device.send_config()
   ```

   Config values can be `True`, `False`, or `None`. `None` uses the device default value. Call `device.config.reset()` to reset all fields to `None`.

   | Device | Config field | Description |
   | ------ | ------------ | ----------- |
   | N4Pro | `led_follow_key_light` | Link RGB LED state with key light state |
   | N4Pro | `key_light_on_disconnect` | Keep key light behavior enabled after disconnect |
   | N4Pro | `check_usb_power` | Enable USB power checking |
   | N4Pro | `enable_vibration` | Enable vibration feedback |
   | N4Pro | `reset_usb_report` | Enable USB report reset behavior |
   | N4Pro | `enable_boot_video` | Enable boot video |
   | XL | `led_follow_key_light` | Link RGB LED state with key light state |
7. **Listen for key events**

   ```python
   def key_callback(device, key, state):
       # state=1 means pressed, state=0 means released
       pass

   device.set_key_callback(key_callback)
   ```
8. **Listen for N4Pro touch points**

   ```python
   from StreamDock.InputTypes import EventType
   from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro

   def touch_callback(device, event):
       if event.event_type == EventType.TOUCH_POINT:
           print(f"Touch point: ({event.x}, {event.y})")

   if isinstance(device, StreamDockN4Pro):
       device.set_touch_bar_callback(touch_callback)
   ```
9. **Clean up resources**

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

When a device is unplugged and reinserted, `DeviceManager.listen()` will automatically detect the device change:

- Added devices are appended to `manager.streamdocks`
- Removed devices are removed from `manager.streamdocks` and closed with `close()`
- Use `on_device_added` / `on_device_removed` to restore application state

It is recommended to wrap your setup logic in a function and reuse it after reconnection:

```python
def auto_init(device):
    """Device auto-initialization function"""
    device.open()
    device.init()
    device.set_key_image(1, "img/default.png")
    device.refresh()
    device.set_key_callback(key_callback)
    # Other initialization operations...

t = threading.Thread(
    target=manager.listen,
    kwargs={
        "on_device_added": auto_init,
        "on_device_removed": lambda device: print(f"Device removed: {device.path}"),
        "auto_open": False,
    },
)
t.daemon = True
t.start()
```

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) file for details.

## Contributing

Welcome to submit Issues and Pull Requests to improve this project.
