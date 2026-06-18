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

## OEM 设备别名

部分 OEM 设备与已支持型号使用相同的 StreamDock 信令协议，但 USB VID/PID 不同。可以在启动服务时使用可重复的 `-oem` 参数注册：

```bash
WebsocketSDK.exe -oem N4Pro:0x1234:0x5678
```

可以同时注册多个 OEM 设备：

```bash
WebsocketSDK.exe -oem N4Pro:0x1234:0x5678 -oem XL:0x2345:0x6789
```

格式为 `型号:VID:PID`。`VID` 和 `PID` 可以使用十六进制，例如 `0x1234`，也可以使用十进制。OEM 设备必须与所选择的型号使用相同协议。

可用的 `型号` 别名如下，大小写不敏感：

| OEM 别名 | 对应协议型号 |
| -------- | ------------ |
| `293`    | StreamDock 293 |
| `293V3`  | StreamDock 293V3 |
| `293s`   | StreamDock 293s |
| `293sV3` | StreamDock 293sV3 |
| `N3`     | StreamDock N3 |
| `N4`     | StreamDock N4 |
| `N1`     | StreamDock N1 |
| `N4Pro`  | StreamDock N4Pro |
| `XL`     | StreamDock XL |
| `M18`    | StreamDock M18 |
| `M3`     | StreamDock M3 |
| `K1Pro`  | K1Pro |

## Linux 运行依赖

在 Linux 上，预编译的 `StreamDockWebsocketSDK` 可执行文件会动态链接以下系统库：

- `libudev.so.1`
- `libresolv.so.2`
- `libpthread.so.0`
- `libc.so.6`
- 平台动态加载器，例如 arm64 上的 `ld-linux-aarch64.so.1`

Ubuntu / Debian 可在启动服务前安装 udev 运行库：

```bash
sudo apt update
sudo apt install -y libudev1 libhidapi-dev
```

本仓库的 SDK 采用 Ubuntu 20.04 编译,使用的 glibc 版本是 **2.31**.

如果服务因为 Linux 设备权限无法访问 StreamDock 设备，可以使用 `sudo` 运行

```
sudo StreamDockWebsocketSDK
```

或者添加设备权限:

把 99-streamdock.rules 复制到  /etc/udev/rules.d/ 目录.后执行:

```
sudo udevadm control --reload-rules
sudo udevadm trigger
```

对于 OEM 设备，请在重新加载 udev 规则前，为 OEM VID/PID 增加规则。以下示例对应 `0x1234:0x5678`：

```bash
SUBSYSTEM=="usb", ATTR{idVendor}=="1234", ATTR{idProduct}=="5678", MODE="0666", GROUP="plugdev"
KERNEL=="hidraw*", ATTRS{idVendor}=="1234", MODE="0666", GROUP="plugdev"
```

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

### 设置按键 GIF

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

### 分别设置 LED 颜色

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
| knobId    | number | 旋钮 ID                 |
| direction | string | 方向："left" 或 "right" |

**旋钮按下事件 Payload：**

| 参数名 | 类型   | 说明            |
| ------ | ------ | --------------- |
| knobId | number | 旋钮 ID         |
| state  | string | 状态："pressed" 或 "released" |

**滑动手势事件 Payload：**

| 参数名    | 类型   | 说明                    |
| --------- | ------ | ----------------------- |
| direction | string | 方向："left" 或 "right" |

**N4Pro 触摸点事件 Payload：**

| 参数名 | 类型   | 说明                     |
| ------ | ------ | ------------------------ |
| type   | string | 固定为 `"touch_point"` |
| x      | number | 触摸 X 坐标              |
| y      | number | 触摸 Y 坐标              |

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

| 参数名    | 类型   | 必填 | 说明                                                            |
| --------- | ------ | ---- | --------------------------------------------------------------- |
| imagePath | string | 是   | 图像文件完整路径；常见可解码格式会自动转换为设备需要的 JPEG/PNG |

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

| 参数名  | 类型   | 必填 | 说明                                                        |
| ------- | ------ | ---- | ----------------------------------------------------------- |
| imgData | string | 是   | Base64 图像数据或 data URL；会自动转换为设备需要的 JPEG/PNG |

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

#### setTemporaryBackgroundImg - 从文件设置临时背景

使用 BGPIC 帧流设置临时背景，仅 M18 和 N4Pro 支持。

**Payload 参数：**

| 参数名    | 类型   | 必填 | 说明             |
| --------- | ------ | ---- | ---------------- |
| imagePath | string | 是   | 图像文件完整路径 |

#### setTemporaryBackgroundImgData - 从 Base64 设置临时背景

使用 Base64 图像数据设置临时背景，仅 M18 和 N4Pro 支持。

**Payload 参数：**

| 参数名  | 类型   | 必填 | 说明                       |
| ------- | ------ | ---- | -------------------------- |
| imgData | string | 是   | Base64 图像数据或 data URL |

#### setKeyImg - 从文件设置按键图标

从本地文件路径加载并设置指定按键的图标。

**Payload 参数：**

| 参数名    | 类型   | 必填 | 说明                                                            |
| --------- | ------ | ---- | --------------------------------------------------------------- |
| keyIndex  | number | 是   | 按键索引（从 1 开始）                                           |
| imagePath | string | 是   | 图像文件完整路径；常见可解码格式会自动转换为设备需要的 JPEG/PNG |

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

#### setKeyGif - 从文件设置按键 GIF

加载 GIF 文件并绑定到指定按键。GIF 会在调用 `startGifLoop` 后开始播放。

**参数：**

| 参数      | 类型   | 必需 | 说明                                    |
| --------- | ------ | ---- | --------------------------------------- |
| keyIndex  | number | 是   | 按键索引（从 1 开始，根据设备型号不同） |
| imagePath | string | 是   | GIF 文件完整路径                        |

**请求示例：**

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

#### clearKeyGif - 清除按键 GIF

停止并移除指定按键的 GIF。

**参数：**

| 参数     | 类型   | 必需 | 说明                                    |
| -------- | ------ | ---- | --------------------------------------- |
| keyIndex | number | 是   | 按键索引（从 1 开始，根据设备型号不同） |

#### setBackgroundGif - 从文件设置背景 GIF

加载 GIF 文件并绑定到触摸屏背景。该功能仅支持 N4Pro、XL 和 M3 设备。GIF 会在调用 `startGifLoop` 后开始播放。

**参数：**

| 参数      | 类型   | 必需 | 说明             |
| --------- | ------ | ---- | ---------------- |
| imagePath | string | 是   | GIF 文件完整路径 |
| x         | number | 否   | X 坐标，默认 0   |
| y         | number | 否   | Y 坐标，默认 0   |
| fbLayer   | number | 否   | 帧缓冲层，默认 0 |

**请求示例：**

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

#### clearBackgroundGif - 清除背景 GIF

停止并清除背景 GIF 帧流。该功能仅支持 N4Pro、XL 和 M3 设备。

**参数：**

| 参数     | 类型   | 必需 | 说明               |
| -------- | ------ | ---- | ------------------ |
| position | number | 否   | 清除位置值，默认 3 |

#### startGifLoop / stopGifLoop / gifLoopStatus - 控制 GIF 播放

启动、停止或查询按键和背景 GIF 共用的播放循环。

**请求示例：**

```json
{ "event": "startGifLoop", "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==", "payload": {} }
{ "event": "stopGifLoop", "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==", "payload": {} }
{ "event": "gifLoopStatus", "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==", "payload": {} }
```

`gifLoopStatus` 返回：

```json
{
  "event": "gifLoopStatus",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "running": true
  }
}
```

#### setKeyImgBulk - 批量设置按键图标

使用单个图像文件批量设置多个按键的图标。这比多次调用 `setKeyImg` 更高效。

**Payload 参数：**

| 参数名     | 类型     | 必填 | 说明                                                |
| ---------- | -------- | ---- | --------------------------------------------------- |
| imagePath  | string   | 是   | 图像文件的完整路径（支持 jpg、png）                 |
| keyIndexes | number[] | 是   | 要更新的按键索引数组（从 1 开始，根据设备型号不同） |

**请求示例：**

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

#### read - 开始监听设备输入

开始监听设备的按键、旋钮、触摸等输入事件。

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
| brightness | number | 是   | LED 亮度值，范围 0-255 |

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

#### setLEDColor - 设置全部的 LED 颜色

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

#### setSingleLedColor - 分别设置 LED 颜色

按设备 LED 顺序分别设置 RGB 颜色。

**Payload 参数：**

| 参数名 | 类型       | 必填 | 说明                                      |
| ------ | ---------- | ---- | ----------------------------------------- |
| colors | number[][] | 是   | 按 LED 顺序排列的 RGB 三元组；每个分量范围 0-255 |

**请求示例：**

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

#### setDeviceConfig - 设置设备配置

通过设备 QUCMD 协议设置 N4Pro 或 XL 的设备配置。

**支持设备：**

| 设备  | Payload 字段                                                                                                                     |
| ----- | -------------------------------------------------------------------------------------------------------------------------------- |
| N4Pro | `ledFollowKeyLight`, `keyLightOnDisconnect`, `checkUsbPower`, `enableVibration`, `resetUsbReport`, `enableBootVideo` |
| XL    | `ledFollowKeyLight`                                                                                                            |

**Payload 值：**

使用 `true` 表示开启，`false` 表示关闭；省略字段时使用协议默认值。

**请求示例：**

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

### N1 设备专用事件

以下事件仅适用于 N1 设备：

#### changeN1Mode - 改变 N1 设备模式

切换 N1 设备的不同操作模式。

**Payload 参数：**

| 参数名 | 类型   | 必填 | 说明                                         |
| ------ | ------ | ---- | -------------------------------------------- |
| mode   | string | 是   | 设备模式："keyboard"、"calculator" 或 "dock" |

**请求示例：**

```json
{
  "event": "changeN1Mode",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "mode": "keyboard"
  }
}
```

#### changeN1Page - 改变 N1 设备页面

切换 N1 设备的不同页面（1-5 页）。

**Payload 参数：**

| 参数名 | 类型   | 必填 | 说明        |
| ------ | ------ | ---- | ----------- |
| page   | number | 是   | 页码（1-5） |

**请求示例：**

```json
{
  "event": "changeN1Page",
  "path": "XHk6XFxXZW50YnVnZGV2aWNlcw==",
  "payload": {
    "page": 2
  }
}
```

#### setN1SkinBitmap - 从 Base64 设置 N1 皮肤

使用 Base64 编码的图像数据设置 N1 设备皮肤图像。

**Payload 参数：**

| 参数名     | 类型   | 必填 | 说明                                                  |
| ---------- | ------ | ---- | ----------------------------------------------------- |
| skinMode   | string | 是   | 皮肤模式："keyboard"、"keyboard_lock" 或 "calculator" |
| skinPage   | number | 是   | 皮肤页码                                              |
| skinStatus | string | 是   | 皮肤状态："press" 或 "release"                        |
| keyIndex   | number | 是   | 按键索引（从 1 开始）                                 |
| imageData  | string | 是   | Base64 编码的图像数据                                 |

**请求示例：**

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

#### setN1SkinBitmapFromFile - 从文件设置 N1 皮肤

从本地文件路径设置 N1 设备皮肤图像。

**Payload 参数：**

| 参数名     | 类型   | 必填 | 说明                                                  |
| ---------- | ------ | ---- | ----------------------------------------------------- |
| skinMode   | string | 是   | 皮肤模式："keyboard"、"keyboard_lock" 或 "calculator" |
| skinPage   | number | 是   | 皮肤页码                                              |
| skinStatus | string | 是   | 皮肤状态："press" 或 "release"                        |
| keyIndex   | number | 是   | 按键索引（从 1 开始）                                 |
| imagePath  | string | 是   | 图像文件的完整路径（支持 jpg、png）                   |

**请求示例：**

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

### M3 设备专用事件

以下事件仅适用于 M3 设备：

M3 旋钮按压输入也会通过 `read` 事件的旋钮按下 Payload 上报 `"pressed"` 和 `"released"` 状态。

#### magneticCalibration - M3 磁力校准

校准 M3 设备的磁力传感器。

**Payload 参数：** 无（空对象 `{}`）

**请求示例：**

```json
{
  "event": "magneticCalibration",
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
- **图片格式**：支持 JPEG、PNG 和 GIF 文件，静态图片和 GIF 帧会自动缩放到目标尺寸
- **事件顺序**：建议先等待设备连接事件，再进行设备操作
- **错误处理**：建议增加完善的错误处理逻辑以保证稳定性
