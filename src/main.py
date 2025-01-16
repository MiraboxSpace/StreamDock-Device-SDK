from StreamDock.DeviceManager import DeviceManager
from StreamDock.ImageHelpers import PILHelper
import threading
import time
import os

if __name__ == "__main__":
    manner=DeviceManager()
    streamdocks= manner.enumerate()
    print("Found {} Stream Dock(s).\n".format(len(streamdocks)))
    for device in streamdocks:
        # 打开设备
        device.open()
        device.wakeScreen()
        # 开线程获取设备反馈
        t1= threading.Thread(target=device.whileread)
        t1.start()
        # 设置设备亮度0-100
        device.set_brightness(50)
        # # 设置背景图片（传图片的地址）
        device.clearAllIcon()
        device.set_touchscreen_image("../img/YiFei.jpg")
        device.refresh()
        time.sleep(3)
        # device.clearAllIcon()
        for i in range(16):
            device.set_key_image(i + 1, "../img/tiga112.jpg")
        device.refresh()
        time.sleep(3)
        # 清空某个按键的图标
        device.cleaerIcon(3)
        time.sleep(1)
        # 清空所有按键的图标
        device.clearAllIcon()
        device.refresh()
        time.sleep(1)
        # 关闭设备
        # device.close()
        # time.sleep(1)
    t1.join()
    
