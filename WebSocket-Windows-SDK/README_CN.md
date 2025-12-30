# StreamDock WebSocket SDK 使用指南

## 概述

`StreamDock WebSocket SDK` 是一个用于控制 StreamDock 设备的 WebSocket 服务器。通过标准的 WebSocket 协议，开发者可以使用任何编程语言与 StreamDock 设备进行通信，实现设备控制、按键监听、图像显示等功能。

## 特性

- **独立运行**：无需官方软件，直接控制 StreamDock 设备
- **跨平台通信**：基于标准 WebSocket 协议，支持所有编程语言
- **实时交互**：支持设备热插拔、按键输入监听、实时图像更新
- **丰富功能**：屏幕亮度调节、LED 灯效控制、图像显示、键盘灯效等

## 支持的设备

| 设备型号            |
| ------------------- |
| StreamDock 293系列  |
| StreamDock 293s系列 |
| StreamDock N3 系列  |
| StreamDock N4 系列  |
| StreamDock N1 系列  |
| StreamDock N4Pro    |
| StreamDock XL       |
| StreamDock M18      |
| StreamDock M3       |
| K1Pro               |

## 快速开始

### 1. 启动服务器

双击运行 `WebsocketSDK.exe` 即可启动服务器（默认端口：9002）。

你也可以通过命令行指定端口：

```bash
WebsocketSDK.exe -port 9009
```

### 2. 连接到服务器

WebSocket 端点：`ws://127.0.0.1:9002`

### 3. 基本使用示例

#### JavaScript 示例

```javascript
const ws = new WebSocket('ws://127.0.0.1:9002');
let devicePath = null;

ws.onopen = () => {
    console.log("WebSocket 连接已建立");
};

ws.onmessage = (e) => {
    const response = JSON.parse(e.data);

    // 保存设备路径
    if (response.payload && response.payload.path) {
        devicePath = response.path;
    }

    console.log("收到消息：", response);

    // 处理设备连接事件
    if (response.event === "deviceDidConnect") {
        console.log("设备已连接：", response.payload);
    }

    // 处理按键输入事件
    if (response.event === "read") {
        console.log("按键事件：", response.payload);
    }
};

ws.onerror = (error) => {
    console.error("WebSocket 错误：", error);
};

ws.onclose = () => {
    console.log("连接已关闭");
};
```

#### Python 示例

```python
import asyncio
import websockets
import json

async def streamdock_client():
    uri = "ws://127.0.0.1:9002"
    async with websockets.connect(uri) as websocket:
        # 接收消息
        while True:
            message = await websocket.recv()
            data = json.loads(message)
            print(f"收到消息: {data}")

# 运行客户端
asyncio.get_event_loop().run_until_complete(streamdock_client())
```

## 常用操作

### 设置按键图像

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

### 设置背景图像

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

### 设置屏幕亮度

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

### 设置 LED 颜色

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

### 开始监听按键输入

```javascript
const startRead = {
    event: "read",
    path: devicePath,
    payload: {}
};
ws.send(JSON.stringify(startRead));
```

### 刷新屏幕

```javascript
const refresh = {
    event: "refresh",
    path: devicePath,
    payload: {}
};
ws.send(JSON.stringify(refresh));
```

## 消息格式

所有 WebSocket 消息使用 JSON 格式：

```json
{
  "event": "事件名称",
  "path": "Base64编码的设备路径",
  "payload": {}
}
```

### 服务器推送事件

#### deviceDidConnect - 设备连接事件

当设备连接时触发，返回设备完整信息。

**Payload 参数：**

| 参数名          | 类型   | 说明                  |
| --------------- | ------ | --------------------- |
| path            | string | 设备路径              |
| vendorID        | number | 厂商 ID (十六进制)    |
| productID       | number | 产品 ID (十六进制)    |
| serialNumber    | string | 序列号                |
| manufacturer    | string | 制造商                |
| product         | string | 产品名                |
| firmwareVersion | string | 固件版本              |
| releaseNumber   | number | 版本号                |
| type            | string | 设备类型 (如 "N4Pro") |

**响应示例：**

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

#### deviceDidDisconnect - 设备断开事件

当设备断开时触发。

**Payload 参数：**

| 参数名 | 类型   | 说明     |
| ------ | ------ | -------- |
| path   | string | 设备路径 |

#### read - 输入事件

监听按键、旋钮或触摸输入时触发的事件。

**按钮事件 Payload：**

| 参数名         | 类型   | 说明                       |
| -------------- | ------ | -------------------------- |
| keyId          | number | 按键 ID（1-32，从 1 开始） |
| keyUpOrKeyDown | string | 状态："keyDown" 或 "keyUp" |

**示例：**

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

**旋钮旋转事件 Payload：**

| 参数名    | 类型   | 说明                    |
| --------- | ------ | ----------------------- |
| knobId    | number | 旋钮 ID                |
| direction | string | 方向："left" 或 "right" |

**旋钮按下事件 Payload：**

| 参数名 | 类型   | 说明            |
| ------ | ------ | --------------- |
| knobId | number | 旋钮 ID        |
| state  | string | 状态："pressed" |

**滑动手势事件 Payload：**

| 参数名    | 类型   | 说明                    |
| --------- | ------ | ----------------------- |
| direction | string | 方向："left" 或 "right" |

### 客户端发送事件

#### getFirmVersion - 获取固件版本

获取设备的固件版本信息。

**Payload 参数：** 无（空对象 `{}`）

**响应示例：**

```json
{
  "event": "getFirmVersion",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "firmVersion": "1.0.0"
  }
}
```

#### setBrightness - 设置屏幕亮度

设置设备屏幕的亮度级别。

**Payload 参数：**

| 参数名     | 类型   | 必填 | 说明               |
| ---------- | ------ | ---- | ------------------ |
| brightness | number | 是   | 亮度值，范围 0-100 |

**请求示例：**

```json
{
  "event": "setBrightness",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "brightness": 80
  }
}
```

#### setBackgroundImg - 从文件设置背景图

从本地文件路径加载并设置背景图像。

**Payload 参数：**

| 参数名    | 类型   | 必填 | 说明                                |
| --------- | ------ | ---- | ----------------------------------- |
| imagePath | string | 是   | 图像文件的完整路径（支持 jpg、png） |

**请求示例：**

```json
{
  "event": "setBackgroundImg",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "imagePath": "E:/images/background.jpg"
  }
}
```

#### setBackgroundImgData - 从 Base64 数据设置背景图

使用 Base64 编码的图像数据设置背景图。

**Payload 参数：**

| 参数名  | 类型   | 必填 | 说明                  |
| ------- | ------ | ---- | --------------------- |
| imgData | string | 是   | Base64 编码的图像数据 |

**请求示例：**

```json
{
  "event": "setBackgroundImgData",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "imgData": "data:image/jpeg;base64,/9j/4AAQSkZJRg..."
  }
}
```

#### setKeyImg - 从文件设置按键图标

从本地文件路径加载并设置指定按键的图标。

**Payload 参数：**

| 参数名    | 类型   | 必填 | 说明                                |
| --------- | ------ | ---- | ----------------------------------- |
| keyIndex  | number | 是   | 按键索引（从 1 开始）               |
| imagePath | string | 是   | 图像文件的完整路径（支持 jpg、png） |

**请求示例：**

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

#### setKeyImgData - 从 Base64 数据设置按键图标

使用 Base64 编码的图像数据设置指定按键的图标。

**Payload 参数：**

| 参数名   | 类型   | 必填 | 说明                                    |
| -------- | ------ | ---- | --------------------------------------- |
| keyIndex | number | 是   | 按键索引（从 1 开始，根据设备型号不同） |
| imgData  | string | 是   | Base64 编码的图像数据                   |

**请求示例：**

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

#### read - 开始监听设备输入

开始监听设备的按键、旋钮等输入事件。

**Payload 参数：** 无（空对象 `{}`）

**请求示例：**

```json
{
  "event": "read",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

#### clearAllIcon - 清除所有按键图标

清除设备上所有按键的图标。

**Payload 参数：** 无（空对象 `{}`）

**请求示例：**

```json
{
  "event": "clearAllIcon",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

#### clearIcon - 清除指定按键图标

清除指定按键的图标。

**Payload 参数：**

| 参数名   | 类型   | 必填 | 说明                                    |
| -------- | ------ | ---- | --------------------------------------- |
| keyIndex | number | 是   | 按键索引（从 1 开始，根据设备型号不同） |

**请求示例：**

```json
{
  "event": "clearIcon",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "keyIndex": 0
  }
}
```

#### refresh - 刷新屏幕

刷新设备屏幕显示。

**Payload 参数：** 无（空对象 `{}`）

**请求示例：**

```json
{
  "event": "refresh",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

#### setLEDBrightness - 设置 LED 亮度

设置设备 LED 灯的亮度。

**Payload 参数：**

| 参数名     | 类型   | 必填 | 说明                   |
| ---------- | ------ | ---- | ---------------------- |
| brightness | number | 是   | LED 亮度值，范围 0-100 |

**请求示例：**

```json
{
  "event": "setLEDBrightness",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "brightness": 75
  }
}
```

#### setLEDColor - 设置 LED 颜色

设置设备 LED 灯的 RGB 颜色。

**Payload 参数：**

| 参数名 | 类型   | 必填 | 说明                 |
| ------ | ------ | ---- | -------------------- |
| r      | number | 是   | 红色分量，范围 0-255 |
| g      | number | 是   | 绿色分量，范围 0-255 |
| b      | number | 是   | 蓝色分量，范围 0-255 |

**请求示例：**

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

#### resetLEDColor - 重置 LED 颜色

重置设备 LED 灯为默认颜色。

**Payload 参数：** 无（空对象 `{}`）

**请求示例：**

```json
{
  "event": "resetLEDColor",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {}
}
```

### 键盘设备专用事件

以下事件仅适用于键盘设备（如 K1Pro）：

#### setKeyboardLightingEffects - 设置键盘灯效

设置键盘的灯光效果模式。

**Payload 参数：**

| 参数名 | 类型   | 必填 | 说明                              |
| ------ | ------ | ---- | --------------------------------- |
| effect | number | 是   | 灯效模式编号（0-7）, 0 为静态灯效 |

**请求示例：**

```json
{
  "event": "setKeyboardLightingEffects",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "effect": 1
  }
}
```

#### setKeyboardLightingSpeed - 设置键盘灯效速度

设置键盘灯光效果的变化速度。

**Payload 参数：**

| 参数名 | 类型   | 必填 | 说明          |
| ------ | ------ | ---- | ------------- |
| speed  | number | 是   | 速度值（0-7） |

**请求示例：**

```json
{
  "event": "setKeyboardLightingSpeed",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "speed": 128
  }
}
```

#### setKeyboardRGBBacklight - 设置键盘 RGB 背光

设置键盘的 RGB 背光颜色。

**Payload 参数：**

| 参数名 | 类型   | 必填 | 说明                 |
| ------ | ------ | ---- | -------------------- |
| r      | number | 是   | 红色分量，范围 0-255 |
| g      | number | 是   | 绿色分量，范围 0-255 |
| b      | number | 是   | 蓝色分量，范围 0-255 |

**请求示例：**

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

#### setKeyboardOSMode - 设置键盘 OS 模式

设置键盘的操作系统模式（用于适配不同的操作系统）。

**Payload 参数：**

| 参数名 | 类型   | 必填 | 说明                              |
| ------ | ------ | ---- | --------------------------------- |
| osMode | number | 是   | OS 模式编号（0:windows, 1:macOS） |

**请求示例：**

```json
{
  "event": "setKeyboardOSMode",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "osMode": 1
  }
}
```

#### setKeyboardBacklightBrightness - 设置键盘背光亮度

设置键盘背光的亮度。

**Payload 参数：**

| 参数名     | 类型   | 必填 | 说明                 |
| ---------- | ------ | ---- | -------------------- |
| brightness | number | 是   | 背光亮度值，范围 0-6 |

**请求示例：**

```json
{
  "event": "setKeyboardBacklightBrightness",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "brightness": 6
  }
}
```

## 注意事项

- **路径分隔符**：在 Windows 中应使用双反斜杠 `\\` 或正斜杠 `/`
- **设备路径**：所有操作都需要包含设备路径（Base64 编码），从连接事件中获取
- **图片格式**：支持 JPEG、PNG 格式，图片会自动缩放到目标尺寸
- **事件顺序**：建议先等待设备连接事件，再进行设备操作
- **错误处理**：建议增加完善的错误处理逻辑以保证稳定性
