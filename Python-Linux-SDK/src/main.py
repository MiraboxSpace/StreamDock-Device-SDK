from StreamDock.DeviceManager import DeviceManager
from StreamDock.Devices.StreamDockN1 import StreamDockN1
import threading
import time

if __name__ == "__main__":
    manner = DeviceManager()
    streamdocks= manner.enumerate()
    # 监听设备插拔
    t = threading.Thread(target=manner.listen)
    t.daemon = True 
    t.start()
    print("Found {} Stream Dock(s).\n".format(len(streamdocks)))
    for device in streamdocks:
        # 打开设备
        device.open()
        device.init()
        #设置背景图片
        res = device.set_touchscreen_image("../img/YiFei320.png")
        device.refresh()
        time.sleep(2)
        for i in range(1, 19):
            device.set_key_image(i, "../img/tiga64.png")
            device.refresh()
        time.sleep(2)
        # 清空某个按键的图标
        device.cleaerIcon(3)
        device.refresh()
        time.sleep(1)
        # 清空所有按键的图标
        device.clearAllIcon()
        device.refresh()
        time.sleep(0)
        # N1 switch mode
        if isinstance(device, StreamDockN1):
            device.switch_mode(0)
        # 关闭设备
        # device.close()
    time.sleep(10000)