# StreamDock WebSocket SDK User Guide

## Overview

`StreamDock WebSocket SDK` is a WebSocket server for controlling StreamDock devices. Through the standard WebSocket protocol, developers can communicate with StreamDock devices using any programming language, enabling device control, key monitoring, image display, and more.

## Features

- **Standalone Operation**: Control StreamDock devices directly without official software
- **Cross-platform Communication**: Based on standard WebSocket protocol, supports all programming languages
- **Real-time Interaction**: Supports device hot-plug, key input monitoring, real-time image updates
- **Rich Features**: Screen brightness adjustment, LED lighting control, image display, keyboard lighting effects, etc.

## Supported Devices

| Device Model        |
| ------------------- |
| StreamDock 293 Series |
| StreamDock 293s Series |
| StreamDock N3 Series  |
| StreamDock N4 Series  |
| StreamDock N1 Series  |
| StreamDock N4Pro      |
| StreamDock XL         |
| StreamDock M18        |
| StreamDock M3         |
| K1Pro                 |

## Quick Start

### 1. Start the Server

Double-click `WebsocketSDK.exe` to start the server (default port: 9002).

You can also specify a port via command line:

```bash
WebsocketSDK.exe -port 9009
```

### 2. Connect to Server

WebSocket endpoint: `ws://127.0.0.1:9002`

### 3. Basic Usage Examples

#### JavaScript Example

```javascript
const ws = new WebSocket('ws://127.0.0.1:9002');
let devicePath = null;

ws.onopen = () => {
    console.log("WebSocket connected");
};

ws.onmessage = (e) => {
    const response = JSON.parse(e.data);

    // Save device path
    if (response.payload && response.payload.path) {
        devicePath = response.path;
    }

    console.log("Message received:", response);

    // Handle device connection event
    if (response.event === "deviceDidConnect") {
        console.log("Device connected:", response.payload);
    }

    // Handle key input event
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

# Run client
asyncio.get_event_loop().run_until_complete(streamdock_client())
```

## Common Operations

### Set Key Image

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

### Set Background Image

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

### Start Listening for Key Input

```javascript
const startRead = {
    event: "read",
    path: devicePath,
    payload: {}
};
ws.send(JSON.stringify(startRead));
```

### Refresh Screen

```javascript
const refresh = {
    event: "refresh",
    path: devicePath,
    payload: {}
};
ws.send(JSON.stringify(refresh));
```

## Message Format

All WebSocket messages use JSON format:

```json
{
  "event": "event_name",
  "path": "Base64_encoded_device_path",
  "payload": {}
}
```

### Server Push Events

#### deviceDidConnect - Device Connection Event

Triggered when a device is connected, returns complete device information.

**Payload Parameters:**

| Parameter       | Type   | Description                        |
| --------------- | ------ | ---------------------------------- |
| path            | string | Device path                        |
| vendorID        | number | Vendor ID (hexadecimal)            |
| productID       | number | Product ID (hexadecimal)           |
| serialNumber    | string | Serial number                      |
| manufacturer    | string | Manufacturer                       |
| product         | string | Product name                       |
| firmwareVersion | string | Firmware version                   |
| releaseNumber   | number | Version number                     |
| type            | string | Device type (e.g., "N4Pro")        |

**Response Example:**

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

**Payload Parameters:**

| Parameter | Type   | Description   |
| --------- | ------ | ------------- |
| path      | string | Device path   |

#### read - Input Event

Events triggered when listening for button, knob, or touch input.

**Button Event Payload:**

| Parameter      | Type   | Description                                      |
| -------------- | ------ | ------------------------------------------------ |
| keyId          | number | Key ID (1-32, starting from 1)                  |
| keyUpOrKeyDown | string | State: "keyDown" or "keyUp"                      |

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

**Knob Rotation Event Payload:**

| Parameter | Type   | Description                    |
| --------- | ------ | ------------------------------ |
| knobId    | number | Knob ID                        |
| direction | string | Direction: "left" or "right"   |

**Knob Press Event Payload:**

| Parameter | Type   | Description            |
| --------- | ------ | ---------------------- |
| knobId    | number | Knob ID                |
| state     | string | State: "pressed"       |

**Swipe Gesture Event Payload:**

| Parameter | Type   | Description                    |
| --------- | ------ | ------------------------------ |
| direction | string | Direction: "left" or "right"   |

### Client Send Events

#### getFirmVersion - Get Firmware Version

Get the firmware version information of the device.

**Payload Parameters:** None (empty object `{}`)

**Response Example:**

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

Set the brightness level of the device screen.

**Payload Parameters:**

| Parameter  | Type   | Required | Description                  |
| ---------- | ------ | -------- | ---------------------------- |
| brightness | number | Yes      | Brightness value, range 0-100 |

**Request Example:**

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

Load and set background image from a local file path.

**Payload Parameters:**

| Parameter | Type   | Required | Description                                          |
| --------- | ------ | -------- | ---------------------------------------------------- |
| imagePath | string | Yes      | Full path to the image file (supports jpg, png)      |

**Request Example:**

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

Set background image using Base64-encoded image data.

**Payload Parameters:**

| Parameter | Type   | Required | Description                        |
| --------- | ------ | -------- | ---------------------------------- |
| imgData   | string | Yes      | Base64-encoded image data          |

**Request Example:**

```json
{
  "event": "setBackgroundImgData",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "imgData": "data:image/jpeg;base64,/9j/4AAQSkZJRg..."
  }
}
```

#### setKeyImg - Set Key Icon from File

Load and set the icon for a specific key from a local file path.

**Payload Parameters:**

| Parameter | Type   | Required | Description                                          |
| --------- | ------ | -------- | ---------------------------------------------------- |
| keyIndex  | number | Yes      | Key index (starting from 1)                          |
| imagePath | string | Yes      | Full path to the image file (supports jpg, png)      |

**Request Example:**

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

Set the icon for a specific key using Base64-encoded image data.

**Payload Parameters:**

| Parameter | Type   | Required | Description                                      |
| --------- | ------ | -------- | ------------------------------------------------ |
| keyIndex  | number | Yes      | Key index (starting from 1, varies by device)    |
| imgData   | string | Yes      | Base64-encoded image data                        |

**Request Example:**

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

#### read - Start Listening for Device Input

Start listening for device key, knob, and other input events.

**Payload Parameters:** None (empty object `{}`)

**Request Example:**

```json
{
  "event": "read",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

#### clearAllIcon - Clear All Key Icons

Clear icons for all keys on the device.

**Payload Parameters:** None (empty object `{}`)

**Request Example:**

```json
{
  "event": "clearAllIcon",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

#### clearIcon - Clear Specific Key Icon

Clear the icon for a specific key.

**Payload Parameters:**

| Parameter | Type   | Required | Description                                      |
| --------- | ------ | -------- | ------------------------------------------------ |
| keyIndex  | number | Yes      | Key index (starting from 1, varies by device)    |

**Request Example:**

```json
{
  "event": "clearIcon",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "keyIndex": 0
  }
}
```

#### refresh - Refresh Screen

Refresh the device screen display.

**Payload Parameters:** None (empty object `{}`)

**Request Example:**

```json
{
  "event": "refresh",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

#### setLEDBrightness - Set LED Brightness

Set the brightness of the device LED.

**Payload Parameters:**

| Parameter  | Type   | Required | Description                    |
| ---------- | ------ | -------- | ------------------------------ |
| brightness | number | Yes      | LED brightness value, 0-100    |

**Request Example:**

```json
{
  "event": "setLEDBrightness",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "brightness": 75
  }
}
```

#### setLEDColor - Set LED Color

Set the RGB color of the device LED.

**Payload Parameters:**

| Parameter | Type   | Required | Description                    |
| --------- | ------ | -------- | ------------------------------ |
| r         | number | Yes      | Red component, range 0-255     |
| g         | number | Yes      | Green component, range 0-255   |
| b         | number | Yes      | Blue component, range 0-255    |

**Request Example:**

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

#### resetLEDColor - Reset LED Color

Reset the device LED to the default color.

**Payload Parameters:** None (empty object `{}`)

**Request Example:**

```json
{
  "event": "resetLEDColor",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

### Keyboard Device Specific Events

The following events are only applicable to keyboard devices (such as K1Pro):

#### setKeyboardLightingEffects - Set Keyboard Lighting Effects

Set the lighting effect mode of the keyboard.

**Payload Parameters:**

| Parameter | Type   | Required | Description                            |
| --------- | ------ | -------- | -------------------------------------- |
| effect    | number | Yes      | Effect mode number (0-7), 0 is static  |

**Request Example:**

```json
{
  "event": "setKeyboardLightingEffects",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "effect": 1
  }
}
```

#### setKeyboardLightingSpeed - Set Keyboard Lighting Speed

Set the speed of keyboard lighting effects.

**Payload Parameters:**

| Parameter | Type   | Required | Description        |
| --------- | ------ | -------- | ------------------ |
| speed     | number | Yes      | Speed value (0-7)  |

**Request Example:**

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

Set the RGB backlight color of the keyboard.

**Payload Parameters:**

| Parameter | Type   | Required | Description                    |
| --------- | ------ | -------- | ------------------------------ |
| r         | number | Yes      | Red component, range 0-255     |
| g         | number | Yes      | Green component, range 0-255   |
| b         | number | Yes      | Blue component, range 0-255    |

**Request Example:**

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

Set the operating system mode of the keyboard (for adapting to different operating systems).

**Payload Parameters:**

| Parameter | Type   | Required | Description                                    |
| --------- | ------ | -------- | ---------------------------------------------- |
| osMode    | number | Yes      | OS mode number (0: windows, 1: macOS)          |

**Request Example:**

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

Set the brightness of the keyboard backlight.

**Payload Parameters:**

| Parameter  | Type   | Required | Description                      |
| ---------- | ------ | -------- | -------------------------------- |
| brightness | number | Yes      | Backlight brightness value, 0-6  |

**Request Example:**

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

- **Path Separators**: On Windows, use double backslashes `\\` or forward slashes `/`
- **Device Path**: All operations require the device path (Base64 encoded), obtained from the connection event
- **Image Formats**: Supports JPEG and PNG formats; images will be automatically scaled to target dimensions
- **Event Order**: It is recommended to wait for the device connection event before performing device operations
- **Error Handling**: It is recommended to implement comprehensive error handling logic to ensure stability
