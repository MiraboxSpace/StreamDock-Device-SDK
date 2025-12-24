from StreamDock.DeviceManager import DeviceManager
from StreamDock.Devices.StreamDock import StreamDock
from StreamDock.Devices.K1Pro import K1Pro
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro
from StreamDock.InputTypes import EventType
import threading
import time


def key_callback(device, event):
    """
    新的按键回调函数

    Args:
        device: StreamDock 设备实例
        event: InputEvent 事件对象，包含事件类型和详细信息
    """
    try:
        if event.event_type == EventType.BUTTON:
            action = "按下" if event.state == 1 else "释放"
            print(f"按键 {event.key.value} 被{action}", flush=True)
        elif event.event_type == EventType.KNOB_ROTATE:
            print(f"旋钮 {event.knob_id.value} 向 {event.direction.value} 旋转", flush=True)
        elif event.event_type == EventType.KNOB_PRESS:
            action = "按下" if event.state == 1 else "释放"
            print(f"旋钮 {event.knob_id.value} 被{action}", flush=True)
        elif event.event_type == EventType.SWIPE:
            print(f"滑动手势: {event.direction.value}", flush=True)
        # elif event.event_type == EventType.UNKNOWN:
        #     print(f"未知事件{event}", flush=True)
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
    listen_thread = threading.Thread(target=manner.listen)
    listen_thread.daemon = True
    listen_thread.start()

    print("Found {} Stream Dock(s).\n".format(len(streamdocks)))
    for device in streamdocks:
        # 激活类型检查和补全
        if not isinstance(device, StreamDock):
            continue
        try:
            device.open()
            device.init()
        except Exception as e:
            print(f"打开设备失败: {e}")
            import traceback
            traceback.print_exc()
            raise
        print(f"path: {device.path}\nfirmware_version: {device.firmware_version}\nserial_number: {device.serial_number}")

        # 设置背景图片
        # res = device.set_touchscreen_image("img/backgroud_test.png")
        device.refresh()
        time.sleep(2)

        # 设置按键图片
        for i in range(1, 7):
            device.set_key_image(i, "img/button_test.jpg")
            device.refresh()

        # 设置按键事件回调
        device.set_key_callback(key_callback)

        # # 清空某个按键的图标
        # device.clearIcon(3)
        # device.refresh()
        # time.sleep(1)
        # # 清空所有按键的图标
        # device.clearAllIcon()
        # device.refresh()
        # time.sleep(0)

        # N4Pro 设置 LED
        if isinstance(device, StreamDockN4Pro):
            device.set_led_brightness(100)
            device.set_led_color(0, 0, 255)
        #  K1Pro 设置
        if isinstance(device, K1Pro):
            device.set_keyboard_backlight_brightness(6)
            device.set_keyboard_lighting_speed(3)
            device.set_keyboard_lighting_effects(0)  # static
            device.set_keyboard_rgb_backlight(255, 0, 0)
            device.keyboard_os_mode_switch(0)  # windows mode
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
        # CRITICAL: 确保所有设备都被正确关闭，防止 Segmentation fault
        # 按照逆序关闭设备，确保最后打开的设备最先关闭
        for device in reversed(streamdocks):
            try:
                # 先清除回调，避免在关闭过程中触发回调
                device.set_key_callback(None)
                # 给一点时间让读取线程退出循环
                time.sleep(0.1)
                # 关闭设备
                device.close()
                print(f"设备 {device.path} 已关闭")
            except Exception as e:
                print(f"关闭设备时出错: {e}")
                import traceback
                traceback.print_exc()

        # 最后的清理，确保所有 C 资源都已释放
        time.sleep(0.2)
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
