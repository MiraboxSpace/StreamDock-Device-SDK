# Build Your Own StreamDock Button on Linux with an LLM

A general-purpose guide for wiring physical StreamDock keys to *anything* on a Linux
machine — volume controls, launching apps, toggling services, running scripts — using
Claude, ChatGPT, or any other coding LLM to write the glue code for you.

You do **not** need to be a Python expert. This guide gives you (1) a copy-paste prompt
that tells the LLM everything it needs to know about the SDK, and (2) generic worked
examples you can adapt.

---

## What you're building

The [StreamDock Python SDK](https://github.com/MiraboxSpace/StreamDock-Device-SDK) talks
to the device over USB-HID. Your program:

1. Enumerates and opens the device.
2. Draws an image/icon on each physical key.
3. Registers a callback that fires when a key is pressed or released.
4. In that callback, runs whatever you want — a shell command, an API call, a script.

That's the whole loop. Everything below is just filling in step 4.

---

## Prerequisites (one-time setup)

```bash
# Clone the SDK
git clone https://github.com/MiraboxSpace/StreamDock-Device-SDK.git
cd StreamDock-Device-SDK/Python-SDK

# System libraries (Debian/Ubuntu) — install libusb BEFORE hidapi
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0

# Python deps (a virtualenv is strongly recommended; Python 3.10+ preferred)
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

**Let your user run the device without `sudo`.** Copy the udev rules shipped in the repo
and reload them:

```bash
sudo cp 99-streamdock.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Unplug and replug the device after this. To confirm the device is seen:

```bash
lsusb | grep -i mirabox   # or look for the StreamDock vendor entry
```

---

## The LLM prompt (copy-paste this)

Paste the block below into Claude (or any coding LLM), then add your one-line request at
the end, e.g. *"Make key 1 mute/unmute the system audio."* The prompt front-loads the API
surface so the model doesn't hallucinate method names.

> ````
> I'm writing a Python script for a Mirabox StreamDock macro keypad on Linux using the
> official StreamDock-Device-SDK. Help me write a single self-contained script.
>
> Here is the SDK API you must use (do not invent other methods):
>
> from StreamDock.DeviceManager import DeviceManager
> from StreamDock.InputTypes import EventType
>
> manager = DeviceManager()
> devices = manager.enumerate()          # list of device objects
> device.open(); device.init()           # open + initialize before use
> device.image_keys()                    # iterable of valid key indices (skips knobs)
> device.set_key_image(index, "path.png")# draw a PNG/JPEG icon on a key
> device.set_key_gif(index, "anim.gif")  # animated key icon; then device.start_gif_loop()
> device.clearIcon(index)                # clear one key
> device.clearAllIcon()                  # clear every key
> device.refresh()                       # push pending image changes to the screen
> device.set_brightness(percent)         # 0-100 screen/key brightness
> device.set_key_callback(callback)      # register key handler
> device.close()                         # release the device on exit
>
> The key callback signature is:
>
>   def key_callback(device, event):
>       if event.event_type == EventType.BUTTON:
>           pressed = event.state == 1          # 1 = press, 0 = release
>           key_index = event.key.value
>       elif event.event_type == EventType.KNOB_ROTATE:
>           direction = event.direction.value   # for devices with knobs
>       elif event.event_type == EventType.KNOB_PRESS:
>           pressed = event.state == 1
>       elif event.event_type == EventType.SWIPE:
>           direction = event.direction.value
>
> Requirements for the script you write:
> - Enumerate, open, and init the device; print firmware + serial.
> - Run device.listen() in a daemon thread so hotplug (unplug/replug) is handled, and
>   re-apply my key setup via an on_device_added callback.
> - Draw an icon on each key I use and refresh().
> - In the callback, ACT ON PRESS ONLY (event.state == 1) to avoid double-firing.
> - Run external commands with subprocess and never let an exception kill the loop —
>   wrap each action in try/except and print errors.
> - Keep the main thread alive with a sleep loop; on Ctrl+C, clear icons and close()
>   the device cleanly (closing on exit prevents segfaults).
> - Use absolute paths for icons and make them configurable at the top of the file.
>
> My device has <N> keys. The desktop environment is <e.g. GNOME on Wayland / KDE / X11>.
> Audio is managed by <PipeWire / PulseAudio>. 
>
> Now: <describe what you want each key to do>.
> ````

Fill in the angle-bracket fields. The audio/desktop details matter because the *command*
a key runs differs between PipeWire (`wpctl`), PulseAudio (`pactl`), X11 (`xdotool`), and
Wayland.

---

## Generic examples

These are intentionally simple and vendor-neutral. Mix and match them inside the callback.

### Example 1 — Volume up / down / mute (PipeWire)

Most modern distros (Fedora, Ubuntu 22.10+, recent Debian) use PipeWire, which ships
`wpctl`. PulseAudio equivalents are shown in comments.

```python
import subprocess

SINK = "@DEFAULT_AUDIO_SINK@"

def vol_up():
    subprocess.run(["wpctl", "set-volume", SINK, "5%+"])      # pactl: set-sink-volume @DEFAULT_SINK@ +5%
    subprocess.run(["wpctl", "set-volume", SINK, "1.0", "--limit", "1.0"])  # cap at 100%

def vol_down():
    subprocess.run(["wpctl", "set-volume", SINK, "5%-"])      # pactl: set-sink-volume @DEFAULT_SINK@ -5%

def toggle_mute():
    subprocess.run(["wpctl", "set-mute", SINK, "toggle"])     # pactl: set-sink-mute @DEFAULT_SINK@ toggle
```

Wire them to keys 0, 1, 2:

```python
ACTIONS = {
    0: vol_down,
    1: toggle_mute,
    2: vol_up,
}

def key_callback(device, event):
    if event.event_type != EventType.BUTTON or event.state != 1:
        return  # press only
    action = ACTIONS.get(event.key.value)
    if action:
        try:
            action()
        except Exception as e:
            print(f"action failed: {e}", flush=True)
```

### Example 2 — Volume on a rotary knob (devices with knobs)

```python
def key_callback(device, event):
    if event.event_type == EventType.KNOB_ROTATE:
        step = "5%+" if event.direction.value == "right" else "5%-"
        subprocess.run(["wpctl", "set-volume", SINK, step])
    elif event.event_type == EventType.KNOB_PRESS and event.state == 1:
        subprocess.run(["wpctl", "set-mute", SINK, "toggle"])
```

### Example 3 — Media play/pause / next / previous (any player via MPRIS)

`playerctl` controls any MPRIS-compatible player (Spotify, VLC, browsers, mpv):

```python
def key_callback(device, event):
    if event.event_type != EventType.BUTTON or event.state != 1:
        return
    cmd = {
        0: ["playerctl", "previous"],
        1: ["playerctl", "play-pause"],
        2: ["playerctl", "next"],
    }.get(event.key.value)
    if cmd:
        subprocess.run(cmd)
```

### Example 4 — Launch an application

```python
def launch(*cmd):
    # Detach so the app keeps running independently of this script.
    subprocess.Popen(cmd, start_new_session=True,
                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

ACTIONS = {
    0: lambda: launch("firefox"),
    1: lambda: launch("code"),            # VS Code
    2: lambda: launch("gnome-terminal"),
}
```

### Example 5 — Toggle a systemd service / run any script

```python
def toggle_service(name):
    active = subprocess.run(["systemctl", "is-active", "--quiet", name]).returncode == 0
    subprocess.run(["systemctl", "restart" if not active else "stop", name])

ACTIONS = {
    0: lambda: toggle_service("docker"),
    1: lambda: subprocess.run(["/home/me/scripts/backup.sh"]),
}
```

(Service control needs privileges — run the script as the right user, or grant a specific
`sudoers` rule for just that command rather than running the whole script as root.)

---

## Putting it together (minimal skeleton)

This is the smallest complete script. Hand it to the LLM as a starting point, or run it
as-is and swap in your own `ACTIONS`.

```python
#!/usr/bin/env python3
import subprocess
import threading
import time

from StreamDock.DeviceManager import DeviceManager
from StreamDock.InputTypes import EventType

ICON_DIR = "/absolute/path/to/icons"   # put your PNGs here
SINK = "@DEFAULT_AUDIO_SINK@"

ACTIONS = {
    0: lambda: subprocess.run(["wpctl", "set-volume", SINK, "5%-"]),
    1: lambda: subprocess.run(["wpctl", "set-mute",   SINK, "toggle"]),
    2: lambda: subprocess.run(["wpctl", "set-volume", SINK, "5%+"]),
}
ICONS = {0: "vol_down.png", 1: "mute.png", 2: "vol_up.png"}


def draw_keys(device):
    for index, name in ICONS.items():
        device.set_key_image(index, f"{ICON_DIR}/{name}")
    device.refresh()


def key_callback(device, event):
    if event.event_type != EventType.BUTTON or event.state != 1:
        return
    action = ACTIONS.get(event.key.value)
    if not action:
        return
    try:
        action()
    except Exception as e:
        print(f"key {event.key.value} action failed: {e}", flush=True)


def setup(device):
    device.open()
    device.init()
    print(f"firmware={device.firmware_version} serial={device.serial_number}")
    draw_keys(device)
    device.set_key_callback(key_callback)


def main():
    manager = DeviceManager()
    devices = manager.enumerate()

    threading.Thread(
        target=manager.listen,
        kwargs={"on_device_added": setup, "auto_open": False},
        daemon=True,
    ).start()

    for device in devices:
        setup(device)

    print("Running. Ctrl+C to quit.")
    try:
        while True:
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass
    finally:
        for device in reversed(list(manager.streamdocks)):
            try:
                device.set_key_callback(None)
                time.sleep(0.1)
                device.clearAllIcon()
                device.refresh()
                device.close()
            except Exception as e:
                print(f"close error: {e}")


if __name__ == "__main__":
    main()
```

---

## Make it start automatically (systemd user service)

Once your script works, run it on login without a terminal. Create
`~/.config/systemd/user/streamdock.service`:

```ini
[Unit]
Description=StreamDock macro keys
After=graphical-session.target

[Service]
ExecStart=/path/to/StreamDock-Device-SDK/Python-SDK/.venv/bin/python /path/to/your_script.py
Restart=on-failure
RestartSec=2
# So wpctl/playerctl reach the right audio + display session:
Environment=XDG_RUNTIME_DIR=%t

[Install]
WantedBy=default.target
```

Enable it:

```bash
systemctl --user daemon-reload
systemctl --user enable --now streamdock.service
systemctl --user status streamdock.service     # check it's running
journalctl --user -u streamdock.service -f     # watch logs / your print() output
```

---

## Tips for working with the LLM

- **Tell it your exact key count and layout.** "16-key, 4x4" vs "6-key" changes how it
  loops over `image_keys()` and assigns actions.
- **Tell it your audio/display stack** (PipeWire vs PulseAudio, Wayland vs X11). The SDK
  code is identical; only the *command each key runs* changes.
- **Ask for press-only handling.** Without it, every key fires twice (press + release).
- **Ask it to never crash the loop.** Each action wrapped in try/except keeps one bad
  command from killing the whole keypad.
- **Iterate one key at a time.** Get key 0 doing something trivial (`print`), confirm the
  index matches the physical button, then build out from there — physical-to-index
  mapping is the most common surprise.
- **For icons,** ask the LLM to generate simple labeled PNGs with Pillow if you don't have
  artwork, e.g. a colored square with centered text.

---

## Reference

- SDK repo: <https://github.com/MiraboxSpace/StreamDock-Device-SDK>
- Python SDK README (full API, per-device features, hotplug): `Python-SDK/README.md`
- Working sample covering every device type: `Python-SDK/src/main.py`
- `wpctl` (PipeWire): part of `wireplumber`
- `pactl` (PulseAudio): part of `pulseaudio-utils`
- `playerctl` (MPRIS media control): <https://github.com/altdesktop/playerctl>

License: the SDK is MIT-licensed; this guide assumes the same.
