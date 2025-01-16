# MacOS-arm64 platform

You need to install `pillow`, `pyudev`, `threading`, `ctypes`, `time`, `abc`

```bash
pip install pillow
```
Alternatively, the following libraries may already be included in your Python version and do not need to be installed:
```bash
pip install pyudev
```
```bash
pip install threading
```
```bash
pip install ctypes
```
```bash
pip install time
```
```bash
pip install abc
```

When using it, you need to first define a `DeviceManager` class object (device manager class), and then call its `enumerate()` function to traverse the devices and obtain a list of device objects.

```py
manner = DeviceManager();
streamdocks = manner.enumerate();
```

After obtaining the list of device objects, you need to call the `open()` method to open the device before calling other methods to operate on the device.

**Note! When using set_touchscreen_image() and set_key_image(), the image format must be JPEG.**
**Additionally, the set_touchscreen_image() function requires a fixed image size. For example, the size for the 293V3 is 800x480. The set_key_image() function also accepts a fixed size, such as 112x112 for the 293V3. For specific sizes, please refer to the**[documentation](https://creator.key123.vip/en/windows/websocket/events-sent.html#setkeyimg)and[documentation](https://creator.key123.vip/en/windows/websocket/events-sent.html#setbackgroundimg)

```py
for device in streamdocks:
    device.open()
    device.wakeScreen()

    t1= threading.Thread(target=device.whileread)
    t1.start()

    device.set_brightness(50)

    device.clearAllIcon()
    device.set_touchscreen_image("../img/YiFei.jpg")
    device.refresh()
    time.sleep(3)

    for i in range(16):
        device.set_key_image(i + 1, "../img/tiga112.jpg")
    device.refresh()
    time.sleep(3)

    device.cleaerIcon(3)
    time.sleep(1)

    device.clearAllIcon()
    device.refresh()
    time.sleep(1)

    # device.close()
    # time.sleep(1)
```

<!-- Please see the [document](https://creator.key123.vip/en/linux/python/dependency.html) for details -->