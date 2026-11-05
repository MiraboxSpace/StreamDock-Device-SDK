from StreamDock.DeviceManager import DeviceManager
from StreamDock.Devices.StreamDock import StreamDock
from StreamDock.Devices.K1Pro import K1Pro
from StreamDock.Devices.StreamDockM3 import StreamDockM3
from StreamDock.Devices.StreamDockN1 import StreamDockN1
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro
from StreamDock.InputTypes import EventType
from StreamDock.Devices.StreamDockXL import StreamDockXL
from StreamDock.Devices.StreamDockMini import StreamDockMini
from StreamDock.Devices.StreamDockH1Pro import StreamDockH1Pro
import threading
import time

# The H1 Pro firmware restarts (USB re-enumeration) after saving an uploaded
# GIF, so the hotplug listener re-runs setup on the re-added device. Track
# which devices already received the GIF so the re-setup skips the upload
# (uploading again would restart the device once more).
uploaded_gif_devices = set()
h1pro_reconnected = {}


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
        elif event.event_type == EventType.DIP_SWITCH:
            action = "active" if event.state == 1 else "ended"
            if event.direction is None:
                action = "pressed" if event.state == 1 else "released"
                print(f"DIP {event.dip_id.value} {action}", flush=True)
            else:
                print(
                    f"DIP {event.dip_id.value} {event.direction.value} {action}",
                    flush=True,
                )
    except Exception as e:
        print(f"Key callback error: {e}", flush=True)
        import traceback

        traceback.print_exc()


def touch_callback(device, event):
    try:
        if event.event_type == EventType.TOUCH_POINT:
            print(f"N4Pro touch point: ({event.x}, {event.y})", flush=True)
    except Exception as e:
        print(f"Touch callback error: {e}", flush=True)
        import traceback

        traceback.print_exc()


def setup_device(device):
    # Activate type checking and completion
    if not isinstance(device, StreamDock):
        return

    try:
        if not device.open():
            raise RuntimeError(f"Failed to open device path: {device.path}")
        device.init()
    except Exception as e:
        print(f"Failed to open device: {e}")
        import traceback

        traceback.print_exc()
        raise
    print(
        f"path: {device.path}\nfirmware_version: {device.firmware_version}\nserial_number: {device.serial_number}"
    )

    # Set background image
    # the image will be written to the rom.
    # res = device.set_background_image("img/backgroud_test.png")
    device.refresh()
    time.sleep(2)
    # N4Pro special function
    if isinstance(device, StreamDockN4Pro):
        device.set_led_brightness(100)
        # device.set_led_color(0, 0, 255)
        # N4 Pro support control the single led
        device.set_single_led_color(
            [
                (255, 0, 0),
                (0, 0, 255),
                (255, 0, 255),
                (255, 255, 0),
            ]
        )
        # device.set_frame_background("img/backgroud_test2.png")
        device.set_background_gif("img/backgroud_test.gif")
        # device.set_background_mp4("img/bad_apple.mp4")
        # N4Pro supports config
        device.config.enable_vibration = False
        device.send_config()
        device.set_touch_bar_callback(touch_callback)
        time.sleep(2)
    # XL special function
    elif isinstance(device, StreamDockXL):
        # device.set_frame_background("img/backgroud_test2.png")
        device.set_background_gif("img/backgroud_test.gif")
        # device.set_background_mp4("img/bad_apple.mp4")
        # XL supports config
        device.config.led_follow_key_light = True
        device.send_config()
        time.sleep(2)
    # K1Pro special function
    elif isinstance(device, K1Pro):
        device.set_keyboard_backlight_brightness(6)
        device.set_keyboard_lighting_speed(3)
        device.set_keyboard_lighting_effects(0)  # static
        device.set_keyboard_rgb_backlight(255, 0, 0)
        device.keyboard_os_mode_switch(0)  # windows mode
    # N1 special function
    elif isinstance(device, StreamDockN1):
        device.switch_mode(device.DeviceMode.KEYBOARD)
        for page in range(1, 6):
            device.change_page(page)
            time.sleep(1)
        device.switch_mode(device.DeviceMode.CALCULATOR)
        for page in range(1, 6):
            device.change_page(page)
            time.sleep(1)

        # ⚠️ The target page cannot be the same as the current page. ⚠️
        # device.switch_mode(device.DeviceMode.CALCULATOR)
        # device.change_page(2)
        # device.set_n1_skin_bitmap("img/button_test.jpg", device.SkinMode.CALCULATOR, 1, device.SkinStatus.PRESS, 1)
        # device.set_n1_skin_bitmap("img/button_test.jpg", device.SkinMode.CALCULATOR, 1, device.SkinStatus.RELEASE, 1)
        # device.change_page(1)
        # time.sleep(10)

        device.switch_mode(device.DeviceMode.DOCK)
        device.refresh()

    # M3 special function
    elif isinstance(device, StreamDockM3):
        device.set_frame_background("img/backgroud_test2.png")
        # device.set_background_gif("img/backgroud_test.gif")
        # device.set_background_mp4("img/bad_apple.mp4")
        time.sleep(2)
        # device.magnetic_calibration()
    # mini special function
    elif isinstance(device, StreamDockMini):
        # mini only set all colors
        device.set_led_color(0, 0, 255)
    # H1Pro special function
    elif isinstance(device, StreamDockH1Pro):
        # # GIF Upload
        # device_key = device.get_serial_number() or device.getPath()
        # if device_key in uploaded_gif_devices:
        #     # This is the new HID handle after the upload restarted the device.
        #     # Let the original setup call stop using its disconnected handle.
        #     reconnect_event = h1pro_reconnected.get(device_key)
        #     if reconnect_event is not None:
        #         reconnect_event.set()
        #     print("H1Pro special function: GIF already uploaded, skipping upload")
        # else:
        #     print("H1Pro special function: uploading a GIF")
        #     # Upload only: the SDK rotates this 640x360 GIF 90°
        #     # counterclockwise onto a 240x320 canvas (portrait panel) and
        #     # retries lower-quality encodings if the device reports less free
        #     # storage than the encoded file needs (within the 5 MiB file
        #     # limit). Saving the video restarts the firmware, which
        #     # re-enumerates the USB device and triggers the hotplug re-setup;
        #     # the device is marked before uploading so the re-setup (which can
        #     # start while this call is still returning) skips the upload.
        #     reconnect_event = threading.Event()
        #     h1pro_reconnected[device_key] = reconnect_event
        #     uploaded_gif_devices.add(device_key)
        #     try:
        #         device.upload_gif("img/backgroud_test.gif")
        #     except RuntimeError as exc:
        #         # The upload may have committed before the firmware reset the
        #         # USB connection. In that case the old handle times out while
        #         # the hotplug callback has already opened the new handle.
        #         if "0x05000301" in str(exc) and reconnect_event.wait(5):
        #             print("H1Pro special function: upload restarted the device; continuing on the new handle")
        #             return
        #         uploaded_gif_devices.discard(device_key)
        #         raise
        #     except Exception:
        #         uploaded_gif_devices.discard(device_key)
        #         raise
        #     else:
        #         if reconnect_event.wait(3):
        #             # The hotplug callback owns mode and key setup from here on.
        #             return
        #     finally:
        #         h1pro_reconnected.pop(device_key, None)
        # Playback is a separate operation; H1 Pro has no background GIF stream.
        device.switch_mode(device.DeviceMode.GIF)
        time.sleep(5)
        print("H1Pro special function: switching to SCREENSAVER mode")
        device.switch_mode(device.DeviceMode.SCREENSAVER)
        # res = device.set_background_image("img/backgroud_test.png")
        time.sleep(5)
        print("H1Pro special function: switching to KEY mode")
        device.switch_mode(device.DeviceMode.KEY)
        #  for H1Pro,clearAllIcon and refresh is necessary for switching Key mode.
        device.clearAllIcon()
        device.refresh()
        time.sleep(5)

    animated = device.feature_option.supportKeyGif
    for i in device.image_keys():
        if animated and 0 == i % 3:
            device.set_key_gif(i, "img/test.gif")
        elif (i % 3 == 1 if animated else i % 2 == 1):
            device.set_key_image(i, "img/button_test.jpg")
        else:
            device.set_key_image(i, "img/mark.png")

    device.start_animation_loop()
    # Set key event callback
    device.set_key_callback(key_callback)


def device_added_callback(device):
    print(f"[hotplug] Device added: {device.path}", flush=True)
    setup_device(device)


def device_removed_callback(device):
    print(f"[hotplug] Device removed: {device.path}", flush=True)


def main():
    # Wait a moment to ensure previous instances have released resources
    time.sleep(0.5)

    manner = DeviceManager()
    streamdocks = list(manner.enumerate())

    # Listen for device plug/unplug events
    listen_thread = threading.Thread(
        target=manner.listen,
        kwargs={
            "on_device_added": device_added_callback,
            "on_device_removed": device_removed_callback,
            "auto_open": False,
        },
    )
    listen_thread.daemon = True
    listen_thread.start()

    if not streamdocks:
        print("No Stream Dock device found, waiting for hotplug...")
    else:
        print("Found {} Stream Dock(s).\n".format(len(streamdocks)))

    # Hotplug mutates manner.streamdocks; this list is the initial snapshot.
    for device in streamdocks:
        setup_device(device)

        # Clear icon for a specific key
        # device.clearIcon(3)
        # device.refresh()
        # time.sleep(1)

        # # Clear all key icons
        # device.clearAllIcon()
        # device.refresh()

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
        for device in reversed(list(manner.streamdocks)):
            try:
                # Clear callback first to avoid triggering callbacks during shutdown
                device.set_key_callback(None)
                device.set_touchscreen_callback(None)
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
