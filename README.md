# Linux-x86_64 platform
### It is recommended to use Python 3 or above. Test environment: Ubuntu 20.04, Python 3.8.10
  
You need to install `pillow`, `pyudev`, `threading`, `ctypes`, `time`, `abc`
  
```bash
pip install pillow
```
```bash
pip  install  pyudev
``` 

Alternatively, the following libraries may already be included in your Python version and do not need to be installed:
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
  
### Install Linux Software
  
If you want to install them one by one, please pay attention to the installation order: libusb-1.0-0-dev needs to be installed before libhidapi-libusb0.
If you encounter the error "undefined reference to `get_input_report()`", you can replace the `/usr/local/lib/libhidapi-libusb.so.0`file with the`libhidapi-libusb.so.0` file we provided in the Transport folder.
  
```
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0
```
  
When using it, you need to first define a `DeviceManager` class object (device manager class), and then call its `enumerate()` function to traverse the devices and obtain a list of device objects.
  
```py
manner = DeviceManager();
streamdocks = manner.enumerate();
```
  
After obtaining the list of device objects, you need to call the `open()` method to open the device before calling other methods to operate on the device.
  
**~~Note! When using `set_touchscreen_image()` and `set_key_image()`, the image format must be JPEG.~~ **However**, in the new version, you don't need to worry about the image size details. The SDK will handle them for you internally.**
**~~Additionally, the `set_touchscreen_image()` function requires a fixed image size. For example, the size for the 293V3 is 800x480. The `set_key_image()` function also accepts a fixed size, such as 112x112 for the 293V3. For specific sizes, please refer to the~~**[documentation](https://creator.key123.vip/en/windows/websocket/events-sent.html#setkeyimg)and[documentation](https://creator.key123.vip/en/windows/websocket/events-sent.html#setbackgroundimg)
  
```py
from StreamDock.DeviceManager import DeviceManager
from StreamDock.ImageHelpers import PILHelper
import threading
import time

if __name__ == "__main__":
    manner = DeviceManager()
    streamdocks= manner.enumerate()
    # monitor plug-in and unplug devices
    t = threading.Thread(target=manner.listen)
    t.daemon = True # detach
    t.start()
    print("Found {} Stream Dock(s).\n".format(len(streamdocks)))
    for device in streamdocks:
        # open the device
        device.open()
        device.wakeScreen()
        # create a thread to listen content reading from device
        t = threading.Thread(target = device.whileread)
        t.daemon = True # detach
        t.start()
        # set brightness 0-100
        device.set_brightness(50)
        # set background image
        res = device.set_touchscreen_image("../img/YiFei320.png")
        device.refresh()
        time.sleep(2)
        for i in range(1, 19):
            device.set_key_image(i, "../img/tiga64.png")
            device.refresh()
        time.sleep(2)
        # clear a key by number [1, x]
        device.cleaerIcon(3)
        device.refresh()
        time.sleep(1)
        # clear all key
        device.clearAllIcon()
        time.sleep(1)
        # 关close device
        # device.close()
    time.sleep(10000)
```
### notice!
We recommend using our SDK to listen for device plug/unplug events or key press events. When doing so, try to use `threading.Thread.daemon = True` to properly control the lifecycle of the created child threads, ensuring that the main thread's lifecycle is longer than that of the child threads (this is because listening for device events and key presses is almost a dead-loop operation). If you want to modify the logic of the infinite loop and control the start and stop of these two listening events, you can add boolean variables within the functions to control their start or stop.

One more important point to note is that only newer platforms allow **asynchronous** detection of key press events and other device events (device responses). For example, on the **293V3 and N4 **platforms, you can detect key press events while setting `setKeyImg` or `setTouchScreen`. However, asynchronous device response is not supported on older platforms, such as the** 293 and 293s **platforms. On these older platforms, you cannot detect key press events while performing `setKeyImg` or `setTouchScreen`; you must wait for the `setKeyImg` or `setTouchScreen` event to finish before key press listening can be used when the device is idle. This is the difference between new and old platforms.
