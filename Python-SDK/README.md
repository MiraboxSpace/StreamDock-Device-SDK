# StreamDock Python SDK

## Supported Platforms

| Platform           | Support Status | Description                                                 |
| ------------------ | -------------- | ----------------------------------------------------------- |
| Linux (x64, arm64) | ✅ Supported   | Ubuntu 20.04+ recommends using pyudev for device monitoring |
| Windows (x64)      | ✅ Supported   | Supports WMI and polling mode                               |
| macOS (x64, arm64) | ✅ Supported   | Uses polling mode for device monitoring                     |

## Image parameters

Dimensions are width × height **before rotation**. Pillow uses positive angles for counterclockwise rotation: `rotation=90` is counterclockwise, while `rotation=-90` is clockwise. Key images are resized before rotation and flipping.

| Model | Main key / rotation | Secondary image | Boot-logo input / rotation / encoding |
| --- | --- | --- | --- |
| 293 V2 | 100×100 / 180° | — | 800×480 / 180° / RAW BGR888 |
| 293 V3 | 112×112 / 180° | — | 800×480 / 180° / JPEG |
| 293S V2 | 85×85 / 90° CCW | 80×80 | 854×480 / 90° CCW / RAW BGR888 |
| 293S V3 | 96×96 / 90° CCW | 96×96 | 854×480 / 90° CCW / JPEG |
| N3 V2 / V2E | 60×60 / 0° | — | 320×240 / 90° CW / RAW RGB565 |
| N3 V2.5 / V3 | 64×64 / 90° CW | — | 320×240 / 90° CW / JPEG |
| N1 / N1E | 96×96 / 0° | 80×80 | 480×854 / 0° / JPEG |
| N4 / N4E | 112×112 / 180° | 176×112 | 800×480 / 180° / JPEG |
| N4Pro / N4ProE | 112×112 / 180° | 176×112 | 800×480 / 180° / JPEG |
| M3 / M3E | 96×96 / 90° CCW | — | 854×480 / 90° CCW / JPEG |
| XL / XLE | 80×80 / 180° | — | 1024×600 / 180° / JPEG |
| M18 | 64×64 / 0° | — | 480×272 / 0° / JPEG |
| Mini / MiniW | 64×64 / 90° CCW | — | 320×240 / 90° CCW / JPEG |
| K1Pro | 64×64 / 90° CW | — | Unsupported |

Use `device.image_keys()` to enumerate display slots. Secondary images 16–18 on 293S and N1 are display-only; N1's physical buttons and knob use separate input codes. N3 displays slots 1–6 and M18 slots 1–15; their other buttons are input-only. N4 uses slots 1–14, and N4Pro uses 1–4 and 6–15. Secondary images use the main key rotation.

M3, N4Pro and XL encode sources with alpha channels as PNG, otherwise JPEG. Key JPEG quality starts at 100 and decreases by 2, targeting 9000 bytes for static images and 3000 for GIF frames (2040 on XL). PNG targets are 29000 / 20000 bytes. Animated PNG frames and oversized fallback images use ARGB4444 quantization; the final key fallback may still exceed its target.

Static logos are automatically stretched to the model's dimensions before rotation and encoding; images need no manual resizing. JPEG logos start at quality 100 and reject payloads above 522240 bytes. N1 first lowers quality in steps of 10 to target 302080 bytes and requires a firmware suffix version of at least 13. RAW RGB565 uses little-endian byte order. Background animation frames use JPEG quality 60.

`supportKeyGif` indicates key animation availability. 293 V2, 293S V2, N3 V2, `V25.N3` and `V2.M18` firmware use static images. The example cycles JPG, PNG and GIF on supported devices, otherwise alternates JPG and PNG.

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
7. **Listen for input events**

   ```python
   from StreamDock.InputTypes import EventType

   def key_callback(device, event):
       if event.event_type == EventType.BUTTON:
           print(f"Key {event.key.value}: {event.state}")
       elif event.event_type == EventType.DIP_SWITCH:
           # XL and Mini expose two three-position DIP switches.
           direction = event.direction.value if event.direction else "center"
           print(f"DIP {event.dip_id.value}: {direction}, state={event.state}")

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
