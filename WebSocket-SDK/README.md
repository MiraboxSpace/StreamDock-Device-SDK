# StreamDock WebSocket SDK User Guide

## Overview

`StreamDock WebSocket SDK` is a WebSocket server for controlling StreamDock devices. Through the standard WebSocket protocol, developers can communicate with StreamDock devices from any programming language to control devices, listen for key events, display images, and more.

## Features

- **Standalone operation**: Control StreamDock devices directly without the official software
- **Cross-platform communication**: Built on the standard WebSocket protocol and usable from any programming language
- **Real-time interaction**: Supports device hot-plugging, key input monitoring, and real-time image updates
- **Rich functionality**: Screen brightness adjustment, LED lighting control, image display, keyboard lighting effects, and more

## Supported Devices

| Device Model        |
| ------------------- |
| StreamDock 293 series |
| StreamDock 293s series |
| StreamDock N3 series |
| StreamDock N4 series |
| StreamDock N1 series |
| StreamDock N4Pro    |
| StreamDock XL       |
| StreamDock M18      |
| StreamDock M3       |
| K1Pro               |

## OEM Device Aliases

Some OEM devices use the same StreamDock protocol as a supported model, but expose different USB VID/PID values. Register those devices at startup with the repeatable `-oem` option:

```bash
WebsocketSDK.exe -oem N4Pro:0x1234:0x5678
```

You can register multiple OEM devices:

```bash
WebsocketSDK.exe -oem N4Pro:0x1234:0x5678 -oem XL:0x2345:0x6789
```

The format is `Model:VID:PID`. `VID` and `PID` can be hexadecimal, such as `0x1234`, or decimal. The OEM device must use the same protocol as the selected model.

Available `Model` aliases are case-insensitive:

| OEM alias | Protocol model |
| --------- | -------------- |
| `293`     | StreamDock 293 |
| `293V3`   | StreamDock 293V3 |
| `293s`    | StreamDock 293s |
| `293sV3`  | StreamDock 293sV3 |
| `N3`      | StreamDock N3 |
| `N4`      | StreamDock N4 |
| `N1`      | StreamDock N1 |
| `N4Pro`   | StreamDock N4Pro |
| `XL`      | StreamDock XL |
| `M18`     | StreamDock M18 |
| `M3`      | StreamDock M3 |
| `K1Pro`   | K1Pro |

## Linux Runtime Dependencies

On Linux, the prebuilt `StreamDockWebsocketSDK` executable dynamically links the following system libraries:

- `libudev.so.1`
- `libresolv.so.2`
- `libpthread.so.0`
- `libc.so.6`
- A platform dynamic loader, such as `ld-linux-aarch64.so.1` on arm64

On Ubuntu / Debian, install the udev runtime libraries before starting the service:

```bash
sudo apt update
sudo apt install -y libudev1 libhidapi-dev
```

The SDK in this repository is built on Ubuntu 20.04 and uses glibc **2.31**.

If the service cannot access StreamDock devices because of Linux device permissions, run it with `sudo`:

```bash
sudo StreamDockWebsocketSDK
```

Alternatively, add device permissions:

Copy `99-streamdock.rules` to `/etc/udev/rules.d/`, then run:

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

For OEM devices, add rules for the OEM VID/PID before reloading udev rules. Example for `0x1234:0x5678`:

```bash
SUBSYSTEM=="usb", ATTR{idVendor}=="1234", ATTR{idProduct}=="5678", MODE="0666", GROUP="plugdev"
KERNEL=="hidraw*", ATTRS{idVendor}=="1234", MODE="0666", GROUP="plugdev"
```

## Quick Start

### 1. Start the Server

Double-click `WebsocketSDK.exe` to start the server. The default port is `9002`.

You can also specify a port from the command line:

```bash
WebsocketSDK.exe -port 9009
```

### 2. Connect to the Server

WebSocket endpoint: `ws://127.0.0.1:9002`

### 3. Basic Usage Examples

#### JavaScript Example

```javascript
const ws = new WebSocket('ws://127.0.0.1:9002');
let devicePath = null;

ws.onopen = () => {
    console.log("WebSocket connection established");
};

ws.onmessage = (e) => {
    const response = JSON.parse(e.data);

    // Save the device path
    if (response.payload && response.payload.path) {
        devicePath = response.path;
    }

    console.log("Message received:", response);

    // Handle device connection events
    if (response.event === "deviceDidConnect") {
        console.log("Device connected:", response.payload);
    }

    // Handle key input events
    if (response.event === "read") {
        console.log("Key event:", response.payload);
    }
};

ws.onerror = (error) => {
    console.error("WebSocket error:", error);
};

ws.onclose = () => {
    console.log("Connection closed");
};
```

#### Python Example

```python
import asyncio
import websockets
import json

async def streamdock_client():
    uri = "ws://127.0.0.1:9002"
    async with websockets.connect(uri) as websocket:
        # Receive messages
        while True:
            message = await websocket.recv()
            data = json.loads(message)
            print(f"Message received: {data}")

# Run the client
asyncio.get_event_loop().run_until_complete(streamdock_client())
```

## Common Operations

### Set a Key Image

```javascript
const setKeyImage = {
    event: "setKeyImg",
    path: devicePath,
    payload: {
        keyIndex: 1,
        imagePath: "E:/images/icon.png"
    }
};
ws.send(JSON.stringify(setKeyImage));
```

### Set the Background Image

```javascript
const setBackgroundImage = {
    event: "setBackgroundImg",
    path: devicePath,
    payload: {
        imagePath: "E:/images/background.jpg"
    }
};
ws.send(JSON.stringify(setBackgroundImage));
```

### Set a Key GIF

```javascript
const setKeyGif = {
    event: "setKeyGif",
    path: devicePath,
    payload: {
        keyIndex: 1,
        imagePath: "E:/images/icon.gif"
    }
};
ws.send(JSON.stringify(setKeyGif));
ws.send(JSON.stringify({ event: "startGifLoop", path: devicePath, payload: {} }));
```

### Set Screen Brightness

```javascript
const setBrightness = {
    event: "setBrightness",
    path: devicePath,
    payload: {
        brightness: 80  // 0-100
    }
};
ws.send(JSON.stringify(setBrightness));
```

### Set LED Color

```javascript
const setLEDColor = {
    event: "setLEDColor",
    path: devicePath,
    payload: {
        r: 255,
        g: 0,
        b: 0
    }
};
ws.send(JSON.stringify(setLEDColor));
```

### Set Individual LED Colors

```javascript
const setSingleLedColor = {
    event: "setSingleLedColor",
    path: devicePath,
    payload: {
        colors: [
            [255, 0, 0],
            [0, 255, 0],
            [0, 0, 255]
        ]
    }
};
ws.send(JSON.stringify(setSingleLedColor));
```

### Start Listening for Key Input

```javascript
const startRead = {
    event: "read",
    path: devicePath,
    payload: {}
};
ws.send(JSON.stringify(startRead));
```

### Refresh the Screen

```javascript
const refresh = {
    event: "refresh",
    path: devicePath,
    payload: {}
};
ws.send(JSON.stringify(refresh));
```

## Message Format

All WebSocket messages use JSON:

```json
{
  "event": "event name",
  "path": "Base64-encoded device path",
  "payload": {}
}
```

### Server Push Events

#### deviceDidConnect - Device Connection Event

Triggered when a device is connected. Returns complete device information.

**Payload parameters:**

| Parameter       | Type   | Description                         |
| --------------- | ------ | ----------------------------------- |
| path            | string | Device path                         |
| vendorID        | number | Vendor ID (hexadecimal)             |
| productID       | number | Product ID (hexadecimal)            |
| serialNumber    | string | Serial number                       |
| manufacturer    | string | Manufacturer                        |
| product         | string | Product name                        |
| firmwareVersion | string | Firmware version                    |
| releaseNumber   | number | Release number                      |
| type            | string | Device type, such as `"N4Pro"`      |

**Response example:**

```json
{
  "event": "deviceDidConnect",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "path": "\\\\?\\hid#vid_33db&pid_0065...",
    "vendorID": 13307,
    "productID": 101,
    "serialNumber": "123456",
    "manufacturer": "StreamDock",
    "product": "StreamDock N4Pro",
    "firmwareVersion": "1.0.0",
    "releaseNumber": 256,
    "type": "N4Pro"
  }
}
```

#### deviceDidDisconnect - Device Disconnection Event

Triggered when a device is disconnected.

**Payload parameters:**

| Parameter | Type   | Description |
| --------- | ------ | ----------- |
| path      | string | Device path |

#### read - Input Event

Triggered when listening for key, knob, or touch input.

**Button event payload:**

| Parameter      | Type   | Description                              |
| -------------- | ------ | ---------------------------------------- |
| keyId          | number | Key ID (1-32, starting from 1)           |
| keyUpOrKeyDown | string | State: `"keyDown"` or `"keyUp"`          |

**Example:**

```json
{
  "event": "read",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "keyId": 1,
    "keyUpOrKeyDown": "keyDown"
  }
}
```

**Knob rotation event payload:**

| Parameter | Type   | Description                    |
| --------- | ------ | ------------------------------ |
| knobId    | number | Knob ID                        |
| direction | string | Direction: `"left"` or `"right"` |

**Knob press event payload:**

| Parameter | Type   | Description        |
| --------- | ------ | ------------------ |
| knobId    | number | Knob ID            |
| state     | string | State: `"pressed"` or `"released"` |

**Swipe gesture event payload:**

| Parameter | Type   | Description                      |
| --------- | ------ | -------------------------------- |
| direction | string | Direction: `"left"` or `"right"` |

**N4Pro touch point event payload:**

| Parameter | Type   | Description                    |
| --------- | ------ | ------------------------------ |
| type      | string | Fixed value: `"touch_point"`   |
| x         | number | Touch X coordinate             |
| y         | number | Touch Y coordinate             |

### Client Events

#### getFirmVersion - Get Firmware Version

Gets the device firmware version.

**Payload parameters:** None, use an empty object `{}`.

**Response example:**

```json
{
  "event": "getFirmVersion",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "firmVersion": "1.0.0"
  }
}
```

#### setBrightness - Set Screen Brightness

Sets the device screen brightness level.

**Payload parameters:**

| Parameter  | Type   | Required | Description             |
| ---------- | ------ | -------- | ----------------------- |
| brightness | number | Yes      | Brightness value, 0-100 |

**Request example:**

```json
{
  "event": "setBrightness",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "brightness": 80
  }
}
```

#### setBackgroundImg - Set Background Image from File

Loads and sets the background image from a local file path.

**Payload parameters:**

| Parameter | Type   | Required | Description                                                                        |
| --------- | ------ | -------- | ---------------------------------------------------------------------------------- |
| imagePath | string | Yes      | Full image file path; common decodable formats are converted to the required JPEG/PNG |

**Request example:**

```json
{
  "event": "setBackgroundImg",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "imagePath": "E:/images/background.jpg"
  }
}
```

#### setBackgroundImgData - Set Background Image from Base64 Data

Sets the background image using Base64-encoded image data.

**Payload parameters:**

| Parameter | Type   | Required | Description                                                                 |
| --------- | ------ | -------- | --------------------------------------------------------------------------- |
| imgData   | string | Yes      | Base64 image data or data URL; converted to the required JPEG/PNG           |

**Request example:**

```json
{
  "event": "setBackgroundImgData",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "imgData": "data:image/jpeg;base64,/9j/4AAQSkZJRg..."
  }
}
```

#### setTemporaryBackgroundImg - Set Temporary Background from File

Sets a temporary background using a BGPIC frame stream. Supported only on M18 and N4Pro.

**Payload parameters:**

| Parameter | Type   | Required | Description          |
| --------- | ------ | -------- | -------------------- |
| imagePath | string | Yes      | Full image file path |

#### setTemporaryBackgroundImgData - Set Temporary Background from Base64

Sets a temporary background using Base64 image data. Supported only on M18 and N4Pro.

**Payload parameters:**

| Parameter | Type   | Required | Description               |
| --------- | ------ | -------- | ------------------------- |
| imgData   | string | Yes      | Base64 image data or data URL |

#### setKeyImg - Set Key Icon from File

Loads and sets the icon for a specified key from a local file path.

**Payload parameters:**

| Parameter | Type   | Required | Description                                                                        |
| --------- | ------ | -------- | ---------------------------------------------------------------------------------- |
| keyIndex  | number | Yes      | Key index, starting from 1                                                         |
| imagePath | string | Yes      | Full image file path; common decodable formats are converted to the required JPEG/PNG |

**Request example:**

```json
{
  "event": "setKeyImg",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "keyIndex": 1,
    "imagePath": "E:/images/icon.png"
  }
}
```

#### setKeyImgData - Set Key Icon from Base64 Data

Sets the icon for a specified key using Base64-encoded image data.

**Payload parameters:**

| Parameter | Type   | Required | Description                                      |
| --------- | ------ | -------- | ------------------------------------------------ |
| keyIndex  | number | Yes      | Key index, starting from 1; varies by device model |
| imgData   | string | Yes      | Base64-encoded image data                        |

**Request example:**

```json
{
  "event": "setKeyImgData",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "keyIndex": 1,
    "imgData": "data:image/png;base64,iVBORw0KGgo..."
  }
}
```

#### setKeyGif - Set Key GIF from File

Loads a GIF file and binds it to a specified key. The GIF starts playing after `startGifLoop` is called.

**Parameters:**

| Parameter | Type   | Required | Description                                      |
| --------- | ------ | -------- | ------------------------------------------------ |
| keyIndex  | number | Yes      | Key index, starting from 1; varies by device model |
| imagePath | string | Yes      | Full GIF file path                               |

**Request example:**

```json
{
  "event": "setKeyGif",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "keyIndex": 1,
    "imagePath": "E:/images/icon.gif"
  }
}
```

#### clearKeyGif - Clear Key GIF

Stops and removes the GIF from a specified key.

**Parameters:**

| Parameter | Type   | Required | Description                                      |
| --------- | ------ | -------- | ------------------------------------------------ |
| keyIndex  | number | Yes      | Key index, starting from 1; varies by device model |

#### setBackgroundGif - Set Background GIF from File

Loads a GIF file and binds it to the touchscreen background. This feature is supported only on N4Pro, XL, and M3 devices. The GIF starts playing after `startGifLoop` is called.

**Parameters:**

| Parameter | Type   | Required | Description              |
| --------- | ------ | -------- | ------------------------ |
| imagePath | string | Yes      | Full GIF file path       |
| x         | number | No       | X coordinate, default 0  |
| y         | number | No       | Y coordinate, default 0  |
| fbLayer   | number | No       | Framebuffer layer, default 0 |

**Request example:**

```json
{
  "event": "setBackgroundGif",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "imagePath": "E:/images/background.gif",
    "x": 0,
    "y": 0,
    "fbLayer": 0
  }
}
```

#### clearBackgroundGif - Clear Background GIF

Stops and clears the background GIF frame stream. This feature is supported only on N4Pro, XL, and M3 devices.

**Parameters:**

| Parameter | Type   | Required | Description                    |
| --------- | ------ | -------- | ------------------------------ |
| position  | number | No       | Clear position value, default 3 |

#### startGifLoop / stopGifLoop / gifLoopStatus - Control GIF Playback

Starts, stops, or queries the playback loop shared by key and background GIFs.

**Request examples:**

```json
{ "event": "startGifLoop", "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==", "payload": {} }
{ "event": "stopGifLoop", "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==", "payload": {} }
{ "event": "gifLoopStatus", "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==", "payload": {} }
```

`gifLoopStatus` returns:

```json
{
  "event": "gifLoopStatus",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "running": true
  }
}
```

#### setKeyImgBulk - Set Key Icons in Bulk

Sets icons for multiple keys using a single image file. This is more efficient than calling `setKeyImg` multiple times.

**Payload parameters:**

| Parameter  | Type     | Required | Description                                      |
| ---------- | -------- | -------- | ------------------------------------------------ |
| imagePath  | string   | Yes      | Full image file path; supports JPG and PNG       |
| keyIndexes | number[] | Yes      | Key indexes to update, starting from 1; varies by device model |

**Request example:**

```json
{
  "event": "setKeyImgBulk",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "imagePath": "E:/icons.png",
    "keyIndexes": [1, 2, 3, 4]
  }
}
```

#### read - Start Listening for Device Input

Starts listening for device input events, including keys, knobs, and touch.

**Payload parameters:** None, use an empty object `{}`.

**Request example:**

```json
{
  "event": "read",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

#### clearAllIcon - Clear All Key Icons

Clears all key icons on the device.

**Payload parameters:** None, use an empty object `{}`.

**Request example:**

```json
{
  "event": "clearAllIcon",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

#### clearIcon - Clear a Specified Key Icon

Clears the icon for a specified key.

**Payload parameters:**

| Parameter | Type   | Required | Description                                      |
| --------- | ------ | -------- | ------------------------------------------------ |
| keyIndex  | number | Yes      | Key index, starting from 1; varies by device model |

**Request example:**

```json
{
  "event": "clearIcon",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "keyIndex": 0
  }
}
```

#### refresh - Refresh the Screen

Refreshes the device screen display.

**Payload parameters:** None, use an empty object `{}`.

**Request example:**

```json
{
  "event": "refresh",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

#### setLEDBrightness - Set LED Brightness

Sets the brightness of the device LEDs.

**Payload parameters:**

| Parameter  | Type   | Required | Description                 |
| ---------- | ------ | -------- | --------------------------- |
| brightness | number | Yes      | LED brightness value, 0-255 |

**Request example:**

```json
{
  "event": "setLEDBrightness",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "brightness": 75
  }
}
```

#### setLEDColor - Set All LED Colors

Sets the RGB color of the device LEDs.

**Payload parameters:**

| Parameter | Type   | Required | Description             |
| --------- | ------ | -------- | ----------------------- |
| r         | number | Yes      | Red component, 0-255    |
| g         | number | Yes      | Green component, 0-255  |
| b         | number | Yes      | Blue component, 0-255   |

**Request example:**

```json
{
  "event": "setLEDColor",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "r": 255,
    "g": 0,
    "b": 0
  }
}
```

#### setSingleLedColor - Set Individual LED Colors

Sets RGB colors for LEDs in device LED order.

**Payload parameters:**

| Parameter | Type       | Required | Description                                      |
| --------- | ---------- | -------- | ------------------------------------------------ |
| colors    | number[][] | Yes      | RGB triples in LED order; each component is 0-255 |

**Request example:**

```json
{
  "event": "setSingleLedColor",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "colors": [
      [255, 0, 0],
      [0, 255, 0],
      [0, 0, 255]
    ]
  }
}
```

#### resetLEDColor - Reset LED Color

Resets the device LEDs to the default color.

**Payload parameters:** None, use an empty object `{}`.

**Request example:**

```json
{
  "event": "resetLEDColor",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

#### setDeviceConfig - Set Device Configuration

Sets device configuration for N4Pro or XL through the device QUCMD protocol.

**Supported devices:**

| Device | Payload fields                                                                                                                     |
| ------ | ---------------------------------------------------------------------------------------------------------------------------------- |
| N4Pro  | `ledFollowKeyLight`, `keyLightOnDisconnect`, `checkUsbPower`, `enableVibration`, `resetUsbReport`, `enableBootVideo` |
| XL     | `ledFollowKeyLight`                                                                                                                |

**Payload values:**

Use `true` to enable and `false` to disable. Omitted fields use the protocol default values.

**Request examples:**

```json
{
  "event": "setDeviceConfig",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "enableVibration": false,
    "enableBootVideo": false
  }
}
```

```json
{
  "event": "setDeviceConfig",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "ledFollowKeyLight": true
  }
}
```

### N1 Device-Specific Events

The following events apply only to N1 devices:

#### changeN1Mode - Change N1 Device Mode

Switches the N1 device between different operation modes.

**Payload parameters:**

| Parameter | Type   | Required | Description                                           |
| --------- | ------ | -------- | ----------------------------------------------------- |
| mode      | string | Yes      | Device mode: `"keyboard"`, `"calculator"`, or `"dock"` |

**Request example:**

```json
{
  "event": "changeN1Mode",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "mode": "keyboard"
  }
}
```

#### changeN1Page - Change N1 Device Page

Switches the N1 device between pages 1-5.

**Payload parameters:**

| Parameter | Type   | Required | Description       |
| --------- | ------ | -------- | ----------------- |
| page      | number | Yes      | Page number, 1-5  |

**Request example:**

```json
{
  "event": "changeN1Page",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "page": 2
  }
}
```

#### setN1SkinBitmap - Set N1 Skin from Base64

Sets an N1 device skin image using Base64-encoded image data.

**Payload parameters:**

| Parameter  | Type   | Required | Description                                                   |
| ---------- | ------ | -------- | ------------------------------------------------------------- |
| skinMode   | string | Yes      | Skin mode: `"keyboard"`, `"keyboard_lock"`, or `"calculator"` |
| skinPage   | number | Yes      | Skin page number                                              |
| skinStatus | string | Yes      | Skin state: `"press"` or `"release"`                          |
| keyIndex   | number | Yes      | Key index, starting from 1                                    |
| imageData  | string | Yes      | Base64-encoded image data                                     |

**Request example:**

```json
{
  "event": "setN1SkinBitmap",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "skinMode": "keyboard",
    "skinPage": 1,
    "skinStatus": "press",
    "keyIndex": 1,
    "imageData": "data:image/png;base64,iVBORw0KGgo..."
  }
}
```

#### setN1SkinBitmapFromFile - Set N1 Skin from File

Sets an N1 device skin image from a local file path.

**Payload parameters:**

| Parameter  | Type   | Required | Description                                                   |
| ---------- | ------ | -------- | ------------------------------------------------------------- |
| skinMode   | string | Yes      | Skin mode: `"keyboard"`, `"keyboard_lock"`, or `"calculator"` |
| skinPage   | number | Yes      | Skin page number                                              |
| skinStatus | string | Yes      | Skin state: `"press"` or `"release"`                          |
| keyIndex   | number | Yes      | Key index, starting from 1                                    |
| imagePath  | string | Yes      | Full image file path; supports JPG and PNG                    |

**Request example:**

```json
{
  "event": "setN1SkinBitmapFromFile",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "skinMode": "keyboard",
    "skinPage": 1,
    "skinStatus": "press",
    "keyIndex": 1,
    "imagePath": "E:/skins/keyboard_skin.png"
  }
}
```

### M3 Device-Specific Events

The following events apply only to M3 devices:

M3 knob press input also reports both `"pressed"` and `"released"` states through the `read` event's knob press payload.

#### magneticCalibration - M3 Magnetic Calibration

Calibrates the magnetic sensor of an M3 device.

**Payload parameters:** None, use an empty object `{}`.

**Request example:**

```json
{
  "event": "magneticCalibration",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

### Keyboard Device-Specific Events

The following events apply only to keyboard devices, such as K1Pro:

#### setKeyboardLightingEffects - Set Keyboard Lighting Effects

Sets the lighting effect mode for the keyboard.

**Payload parameters:**

| Parameter | Type   | Required | Description                                      |
| --------- | ------ | -------- | ------------------------------------------------ |
| effect    | number | Yes      | Lighting effect mode ID, 0-7; 0 is static lighting |

**Request example:**

```json
{
  "event": "setKeyboardLightingEffects",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "effect": 1
  }
}
```

#### setKeyboardLightingSpeed - Set Keyboard Lighting Effect Speed

Sets the speed of keyboard lighting effects.

**Payload parameters:**

| Parameter | Type   | Required | Description     |
| --------- | ------ | -------- | --------------- |
| speed     | number | Yes      | Speed value, 0-7 |

**Request example:**

```json
{
  "event": "setKeyboardLightingSpeed",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "speed": 128
  }
}
```

#### setKeyboardRGBBacklight - Set Keyboard RGB Backlight

Sets the RGB backlight color for the keyboard.

**Payload parameters:**

| Parameter | Type   | Required | Description             |
| --------- | ------ | -------- | ----------------------- |
| r         | number | Yes      | Red component, 0-255    |
| g         | number | Yes      | Green component, 0-255  |
| b         | number | Yes      | Blue component, 0-255   |

**Request example:**

```json
{
  "event": "setKeyboardRGBBacklight",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "r": 255,
    "g": 255,
    "b": 255
  }
}
```

#### setKeyboardOSMode - Set Keyboard OS Mode

Sets the keyboard operating system mode for compatibility with different operating systems.

**Payload parameters:**

| Parameter | Type   | Required | Description                    |
| --------- | ------ | -------- | ------------------------------ |
| osMode    | number | Yes      | OS mode ID, 0: Windows, 1: macOS |

**Request example:**

```json
{
  "event": "setKeyboardOSMode",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "osMode": 1
  }
}
```

#### setKeyboardBacklightBrightness - Set Keyboard Backlight Brightness

Sets the keyboard backlight brightness.

**Payload parameters:**

| Parameter  | Type   | Required | Description                   |
| ---------- | ------ | -------- | ----------------------------- |
| brightness | number | Yes      | Backlight brightness value, 0-6 |

**Request example:**

```json
{
  "event": "setKeyboardBacklightBrightness",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "brightness": 6
  }
}
```

## Notes

- **Path separators**: On Windows, use double backslashes `\\` or forward slashes `/`
- **Device path**: All operations must include the Base64-encoded device path obtained from the connection event
- **Image formats**: JPEG, PNG, and GIF files are supported. Static images and GIF frames are automatically scaled to the target size
- **Event order**: Wait for the device connection event before performing device operations
- **Error handling**: Add complete error handling logic to ensure stability
