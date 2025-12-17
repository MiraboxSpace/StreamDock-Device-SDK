from StreamDock.DeviceManager import DeviceManager
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro
import threading
import time


def key_callback(device, key, state):
    try:
        action = "按下" if state == 1 else "释放"
        print(f"按键{key}被{action}", flush=True)
    except Exception as e:
        print(f"按键回调错误: {e}", flush=True)
        import traceback
        traceback.print_exc()


def main():
    # 等待一下，确保之前的实例释放了资源
    time.sleep(0.5)
    
    manner = DeviceManager()
    streamdocks = manner.enumerate()
    
    if not streamdocks:
        print("未找到 Stream Dock 设备")
        return
    
    # 监听设备插拔
    t = threading.Thread(target=manner.listen)
    t.daemon = True
    t.start()
    
    print("Found {} Stream Dock(s).\n".format(len(streamdocks)))
    
    for device in streamdocks:
        # 打开设备
        try:
            device.open()
            device.init()
        except Exception as e:
            print(f"打开设备失败: {e}")
            import traceback
            traceback.print_exc()
            raise
        print(f"path: {device.path}, firmware_version: {device.firmware_version} serial_number: {device.serial_number}")
        # 设置背景图片
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
        

        # # 清空某个按键的图标
        # device.cleaerIcon(3)
        # device.refresh()
        # time.sleep(1)
        # # 清空所有按键的图标
        # device.clearAllIcon()
        # device.refresh()
        # time.sleep(0)
        
        # # N1 switch mode
        # if isinstance(device, StreamDockN1):
        #     device.switch_mode(0)
        # 关闭设备
        # device.close()
    
    print("程序正在运行,按 Ctrl+C 退出...")
    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\n正在关闭设备...")
    except Exception as e:
        print(f"\n主循环异常: {e}")
        import traceback
        traceback.print_exc()
    finally:
        # 确保所有设备都被正确关闭
        for device in streamdocks:
            try:
                device.close()
            except Exception as e:
                print(f"关闭设备时出错: {e}")
        print("程序已退出")

if __name__ == "__main__":
    try:
        main()
    except SystemExit as e:
        print(f"\n程序被SystemExit终止: {e}")
        import traceback
        traceback.print_exc()
    except Exception as e:
        print(f"\n未捕获的异常: {e}")
        import traceback
        traceback.print_exc()
