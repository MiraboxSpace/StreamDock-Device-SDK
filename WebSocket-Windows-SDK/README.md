# StreamDock WebSocket SDK Guide

## Overview

The `StreamDock WebSocket SDK` allows developers to directly communicate with StreamDock devices via the WebSocket protocol — no official software required.

## Features

- **Standalone Control**: Communicate with devices without the official StreamDock software.
- **Cross-Platform**: Uses standard WebSocket protocol, compatible with many platforms and languages.
- **Real-Time Interaction**: Send and receive device events instantly.

## Getting Started

Before using the SDK, you must start the WebSocket service:

💡 **Double-click `WebsocketSDK.exe` inside the SDK folder to launch.**

Alternatively, you can specify a custom port via command line:

```bash
WebsocketSDK.exe 9009
```

Once started, the SDK will listen on `ws://127.0.0.1:9002` or your specified port for incoming connections.

### 1. Connect to Device

```javascript
const ws = new WebSocket('ws://127.0.0.1:9002');
let path = null;

ws.onopen = () => {
    console.log("Connected");
};

ws.onmessage = (e) => {
    const response = JSON.parse(e.data);
    path = response.path;
    console.log("Message:", response);
};

ws.onerror = (error) => {
    console.error("WebSocket error:", error);
    ws.close();
};

ws.onclose = () => {
    console.log("Connection closed");
};
```

### 2. Set Key Image

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

### 3. Refresh the Display

```javascript
const refresh = {
    event: "refresh",
    path: path
};
ws.send(JSON.stringify(refresh));
```

### 4. Read Device State

```javascript
const readState = {
    event: "read",
    path: path
};
ws.send(JSON.stringify(readState));
```

### 5. Set Background Image

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
**more api please see official docs**
## Notes

- **File paths**: Use double backslashes (`\`) or forward slashes (`/`) for file paths on Windows.
- **Order of operations**: Make sure to receive the device `path` before sending actions.
- **Error handling**: Add error handling for better reliability in production.

## Reference

- Official Docs: [StreamDock WebSocket SDK](https://creator.key123.vip/windows/websocket/overview.html)

---

This guide is based on sample code and the official documentation to help developers quickly get started with the StreamDock WebSocket SDK.
