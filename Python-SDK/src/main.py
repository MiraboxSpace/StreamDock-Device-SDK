from StreamDock.DeviceManager import DeviceManager
from StreamDock.Devices.StreamDock import StreamDock
from StreamDock.Devices.K1Pro import K1Pro
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro
from StreamDock.InputTypes import EventType
import threading
import time


def key_callback(device, event):
    """
    New key callback function

    Args:
        device: StreamDock device instance
        event: InputEvent object containing event type and detailed information
    """
    try:
        if event.event_type == EventType.BUTTON:
            action = "pressed" if event.state == 1 else "released"
            print(f"Key {event.key.value} {action}", flush=True)
        elif event.event_type == EventType.KNOB_ROTATE:
            print(
                f"Knob {event.knob_id.value} rotated {event.direction.value}",
                flush=True,
            )
        elif event.event_type == EventType.KNOB_PRESS:
            action = "pressed" if event.state == 1 else "released"
            print(f"Knob {event.knob_id.value} {action}", flush=True)
        elif event.event_type == EventType.SWIPE:
            print(f"Swipe gesture: {event.direction.value}", flush=True)
        # elif event.event_type == EventType.UNKNOWN:
        #     print(f"Unknown event: {event}", flush=True)
    except Exception as e:
        print(f"Key callback error: {e}", flush=True)
        import traceback

        traceback.print_exc()


def main():
    # Wait a moment to ensure previous instances have released resources
    time.sleep(0.5)

    manner = DeviceManager()
    streamdocks = manner.enumerate()

    if not streamdocks:
        print("No Stream Dock device found")
        return

    # Listen for device plug/unplug events
    listen_thread = threading.Thread(target=manner.listen)
    listen_thread.daemon = True
    listen_thread.start()

    print("Found {} Stream Dock(s).\n".format(len(streamdocks)))
    for device in streamdocks:
        # Activate type checking and completion
        if not isinstance(device, StreamDock):
            continue
        try:
            device.open()
            device.init()
        except Exception as e:
            print(f"Failed to open device: {e}")
            import traceback

            traceback.print_exc()
            raise
        print(
            f"path: {device.path}\nfirmware_version: {device.firmware_version}\nserial_number: {device.serial_number}"
        )

        # Set key event callback
        device.set_key_callback(key_callback)

        # # Clear icon for a specific key
        # device.clearIcon(3)
        # device.refresh()
        # time.sleep(1)
        # # Clear all key icons
        # device.clearAllIcon()
        # device.refresh()
        # time.sleep(0)
        # 293sV3
        if isinstance(device, StreamDock):
            # Set background image
            res = device.set_touchscreen_image("img/backgroud_test.png")
            device.refresh()
            time.sleep(2)
            for i in range(1, 16):
                device.set_key_image(i, "img/button_test.jpg")
                device.refresh()
            for i in range(16, 19):
                device.set_key_image(i, "img/button_test.jpg")
                device.refresh()
        # N4Pro LED settings
        elif isinstance(device, StreamDockN4Pro):
            device.set_led_brightness(100)
            device.set_led_color(0, 0, 255)
        #  K1Pro settings
        elif isinstance(device, K1Pro):
            device.set_keyboard_backlight_brightness(6)
            device.set_keyboard_lighting_speed(3)
            device.set_keyboard_lighting_effects(0)  # static
            device.set_keyboard_rgb_backlight(255, 0, 0)
            device.keyboard_os_mode_switch(0)  # windows mode
            for i in range(1, 7):
                device.set_key_image(i, "img/button_test.jpg")
                device.refresh()
        else:
            for i in range(1, 7):
                device.set_key_image(i, "img/button_test.jpg")
                device.refresh()
        # # N1 switch mode
        # if isinstance(device, StreamDockN1):
        #     device.switch_mode(0)

        # Close device
        # device.close()
    print("Program is running, press Ctrl+C to exit...")
    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nShutting down devices...")
    except Exception as e:
        print(f"\nMain loop error: {e}")
        import traceback

        traceback.print_exc()
    finally:
        # CRITICAL: Ensure all devices are properly closed to prevent Segmentation fault
        # Close devices in reverse order, ensuring the last opened device is closed first
        for device in reversed(streamdocks):
            try:
                # Clear callback first to avoid triggering callbacks during shutdown
                device.set_key_callback(None)
                # Give some time for the read thread to exit the loop
                time.sleep(0.1)
                # Close device
                device.close()
                print(f"Device {device.path} closed")
            except Exception as e:
                print(f"Error closing device: {e}")
                import traceback

                traceback.print_exc()

        # Final cleanup, ensure all C resources are released
        time.sleep(0.2)
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
