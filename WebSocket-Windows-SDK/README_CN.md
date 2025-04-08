# StreamDock WebSocket SDK 使用指南

## 概述

`StreamDock WebSocket SDK` 允许开发者通过 WebSocket 协议直接控制 `StreamDock` 设备，无需依赖官方软件，实现与设备的无缝集成。

## 特性

- **独立控制**：无需官方软件即可直接与 StreamDock 设备通信。
- **跨平台支持**：通过标准的 WebSocket 协议，支持多种编程语言和平台。
- **实时交互**：支持实时发送和接收事件，快速响应用户操作。

## 快速开始

在使用 WebSocket SDK 前，请先启动服务：

💡 **双击启动 WebSocket SDK 文件夹中的 `WebsocketSDK.exe`。**

你也可以使用命令行的方式指定端口启动，例如：

```bash
WebsocketSDK.exe 9009
```

成功启动后，SDK 会监听本地端口（如 `ws://127.0.0.1:9002` 或你指定的端口），供客户端进行连接操作。

以下示例展示了如何使用 WebSocket 与 StreamDock 设备进行基本交互。

### 1. 连接到设备

```javascript
const ws = new WebSocket('ws://127.0.0.1:9002');
let path = null;

ws.onopen = () => {
    console.log("连接已建立");
};

ws.onmessage = (e) => {
    const response = JSON.parse(e.data);
    path = response.path;
    console.log("收到消息：", response);
};

ws.onerror = (error) => {
    console.error("WebSocket 错误：", error);
    ws.close();
};

ws.onclose = () => {
    console.log("连接已关闭");
};
```

### 2. 设置按键图像

```javascript
const setKeyImage = {
    event: "setKeyImg",
    path: path,
    payload: {
        url: "E:/img/example.jpg",
        key: 1,
        refresh: true
    }
};
ws.send(JSON.stringify(setKeyImage));
```

### 3. 刷新设备显示

```javascript
const refresh = {
    event: "refresh",
    path: path
};
ws.send(JSON.stringify(refresh));
```

### 4. 读取设备状态

```javascript
const readState = {
    event: "read",
    path: path
};
ws.send(JSON.stringify(readState));
```

### 5. 设置背景图像

```javascript
const setBackgroundImage = {
    event: "setBackgroundImg",
    path: path,
    payload: {
        url: "E:/img/background.jpg"
    }
};
ws.send(JSON.stringify(setBackgroundImage));
```
**更多的api请查看官网文档**

## 注意事项

- **路径分隔符**：在 Windows 中应使用双反斜杠 `\` 或正斜杠 `/`。
- **事件顺序**：确保先收到带有 `path` 的设备信息再进行操作。
- **错误处理**：建议增加错误处理逻辑以保证稳定性。

## 参考链接

- 官方文档：[StreamDock WebSocket SDK](https://creator.key123.vip/windows/websocket/overview.html)

---

以上内容基于提供的示例代码和官方文档整理，旨在帮助开发者快速上手使用 StreamDock WebSocket SDK。
