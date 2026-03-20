```markdown
<p align="center">  
  <img src="_Images/logo.png" height="128">  
  <h1 align="center">StreamDock Device SDK</h1>  
</p>  
  
<p align="center">  
  <a aria-label="Discordコミュニティに参加" href="https://discord.gg/WvCkKRGavX">  
    <img alt="" src="https://img.shields.io/badge/Discord%20に参加-5865F2.svg?style=for-the-badge&logo=Discord&logoColor=white">  
  </a>  
</p>  
  
## 目次  
  
- [概要](#概要)  
- [対応デバイス](#対応デバイス)  
- [Python SDK](#python-sdk)  
  - [対応プラットフォーム](#対応プラットフォーム-python)  
  - [インストール](#インストール-python)  
  - [クイックスタート](#クイックスタート-python)  
  - [Python APIオブジェクト](#python-apiオブジェクト)  
- [C++ SDK](#c-sdk)  
  - [Linuxインストール](#linuxインストール-c)  
  - [Windowsインストール](#windowsインストール-c)  
  - [macOSインストール](#macosインストール-c)  
  - [C++ APIオブジェクト](#c-apiオブジェクト)  
- [WebSocket SDK](#websocket-sdk)  
  - [インストール](#インストール-websocket)  
  - [WebSocket APIイベント](#websocket-apiイベント)  
- [トラブルシューティング](#トラブルシューティング)  
- [ライセンス](#ライセンス)  
  
---  
  
## 概要  
  
**StreamDock Device SDK**は、コードからStreamDockデバイスを直接制御できるSDKです。プロジェクトは3つの独立したSDKを提供しています：  
  
| SDK | 言語 | プラットフォーム |  
|-----|------|----------------|  
| Python SDK | Python 3.10+ | Linux、Windows、macOS |  
| C++ SDK | C++17 | Linux、Windows、macOS |  
| WebSocket SDK | 任意（WebSocketプロトコル） | Windows |  
  
公式完全ドキュメント：[https://creator.key123.vip/en/guide/get-started.html](https://creator.key123.vip/en/guide/get-started.html)  
  
---  
  
## 対応デバイス  
  
| モデル | シリーズ |  
|--------|----------|  
| StreamDock 293 | 293、293s、293V3、293sV3 |  
| StreamDock N | N1、N3、N4、N4 Pro |  
| StreamDock XL | XL |  
| StreamDock M | M3、M18 |  
| K1 Pro | K1 Pro |  
  
---  
  
## Python SDK  
  
### 対応プラットフォーム (Python)  
  
| プラットフォーム | 状態 | 説明 |  
|----------------|------|------|  
| Linux (x64、arm64) | ✅ 対応 | Ubuntu 20.04+、監視にはpyudev推奨 |  
| Windows (x64) | ✅ 対応 | WMIおよびポーリングモード対応 |  
| macOS (x64、arm64) | ✅ 対応 | 監視にポーリングモード使用 |  
  
### インストール (Python)  
  
#### Linux  
  
> 推奨環境：Ubuntu 20.04 + Python 3.10以上  
  
**1. Python依存関係：**  
  
```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

**2. システム依存関係：**  
```bash  
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0  
```  
⚠️ **重要：** `libusb-1.0-0-dev`パッケージを`libhidapi-libusb0`より先にインストールする必要があります。`sudo`が必要です。  

#### Windows  
  
> 推奨環境：Windows 10/11 + Python 3.10  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

ドライバは通常自動インストールされます。問題がある場合はインストール：  
- [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)  
- [HIDAPI Windows Driver](https://github.com/libusb/hidapi)  

#### macOS  
  
> 推奨環境：macOS 11 + Python 3.10以上  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
brew install hidapi  
```  

### クイックスタート (Python)  

```python
from StreamDock.DeviceManager import DeviceManager  
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro  
import threading  
import time  

def key_callback(device, event):  
    from StreamDock.InputTypes import EventType  
    if event.event_type == EventType.BUTTON:  
        action = "押されました" if event.state == 1 else "離されました"  
        print(f"キー {event.key.value} が{action}", flush=True)  

def main():  
    time.sleep(0.5)  
    manager = DeviceManager()  
    devices = manager.enumerate()  

    if not devices:  
        print("StreamDockデバイスが見つかりません")  
        return  

    # 接続/切断イベントを監視  
    t = threading.Thread(target=manager.listen)  
    t.daemon = True  
    t.start()  

    print(f"{len(devices)}台のデバイスを検出しました。\n")  

    for device in devices:  
        device.open()  
        device.init()  
        print(f"パス: {device.path}, ファームウェア: {device.firmware_version}, シリアル: {device.serial_number}")  

        # 背景画像を設定  
        device.set_touchscreen_image("img/backgroud_test.png")  
        device.refresh()  
        time.sleep(2)  

        # キー画像を設定  
        device.set_key_image(1, "img/button_test.jpg")  
        device.refresh()  

        # LED制御（N4 Proのみ）  
        if isinstance(device, StreamDockN4Pro):  
            device.set_led_brightness(100)  
            device.set_led_color(0, 0, 255)  

        # イベントコールバックを登録  
        device.set_key_callback(key_callback)  

    print("プログラム実行中、Ctrl+Cで終了...")  
    try:  
        while True:  
            time.sleep(0.1)  
    except KeyboardInterrupt:  
        print("\nデバイスを閉じています...")  
    finally:  
        for device in devices:  
            device.close()  
        print("プログラム終了")  

if __name__ == "__main__":  
    main()  
```

### Python APIオブジェクト  

**DeviceManager** (`Python-SDK/src/StreamDock/DeviceManager.py`)  
中央デバイス管理。列挙、接続、ホットプラグ監視を担当。  

| メソッド | 説明 |  
|----------|------|  
| `enumerate() -> list` | 接続されているすべてのStreamDockデバイスを列挙。`StreamDock`オブジェクトのリストを返す。 |  
| `listen()` | ホットプラグ監視（接続/切断）を開始。別スレッドで実行する必要あり。 |  

**StreamDock** (ベース抽象クラス) (`Python-SDK/src/StreamDock/Devices/StreamDock.py`)  
全デバイスのベースクラス。直接インスタンス化しないでください。  

| メソッド | 説明 |  
|----------|------|  
| `open()` | デバイス接続を開く。読み取りスレッドとハートビートを開始。 |  
| `init()` | デバイス初期化：画面を起こす、輝度設定、アイコンクリア、ファームウェア取得。 |  
| `close()` | デバイスを閉じて全リソースを解放。 |  
| `set_key_image(key, path)` | 特定キー画像を設定（インデックスは1から）。 |  
| `set_touchscreen_image(path)` | タッチスクリーン背景画像を設定。 |  
| `set_brightness(percent)` | 画面輝度を設定 (0–100)。 |  
| `refresh()` | デバイスディスプレイを更新。 |  

**特定デバイスクラス**  
各モデルは`StreamDock`を継承し抽象メソッドを実装：  

| クラス | ファイル | モデル |  
|--------|----------|--------|  
| `StreamDock293` | `Devices/StreamDock293.py` | 293 |  
| `StreamDock293V3` | `Devices/StreamDock293V3.py` | 293V3 |  
| `StreamDockN4Pro` | `Devices/StreamDockN4Pro.py` | N4 Pro |  
| `K1Pro` | `Devices/K1Pro.py` | K1 Pro |  

---  

## C++ SDK  
  
### Linuxインストール (C++)  

> 推奨環境：Ubuntu 20.04+、CMake 3.16+、C++17  

1. **システム依存関係：**  
```bash  
sudo apt update  
sudo apt install build-essential cmake  
sudo apt install -y libudev-dev libusb-1.0-0-dev libgif-dev  
```  

2. **OpenCV：**  
```bash  
sudo apt install libopencv-dev  
```  

3. **ビルド：**  
```bash  
cd CPP-SDK  
chmod +x build.sh  
./build.sh  
```  

4. **実行：**  
```bash  
sudo ./bin/main  
```  

**`sudo`を避けるには：** `99-streamdock.rules`を`/etc/udev/rules.d/`にコピーして実行：  
```bash  
sudo udevadm control --reload-rules  
sudo udevadm trigger  
```  

### Windowsインストール (C++)  

> 推奨環境：Visual Studio 2022、C++17  

`bin/`ディレクトリの事前コンパイルライブラリ：  
- `opencv_core4120.dll`  
- `opencv_imgcodecs4120.dll`  
- `opencv_imgproc4120.dll`  
- `hidapi.dll`  

**ビルド：**  
```bash  
cd CPP-SDK  
build.bat  
```  

またはVisual Studio 2022で開いて`Ctrl+B`でビルド。  

### macOSインストール (C++)  

1. **OpenCV準備：**  
```bash  
cd CPP-SDK/ImgProcesser/third_party  
bash link_opencv_symlinks.sh  
```  

2. **ビルド：**  
```bash  
cd CPP-SDK  
chmod +x build.sh  
./build.sh  
```  

---  

## WebSocket SDK  

### インストール (WebSocket)  

`WebsocketSDK.exe`を実行（デフォルトポート：9002）：  
```bash  
WebsocketSDK.exe  
# カスタムポートの場合：  
WebsocketSDK.exe -port 9009  
```  

WebSocket接続：`ws://127.0.0.1:9002`  

**Pythonテスト依存関係：**  
```bash  
pip install websockets  
```  

### WebSocket APIイベント  

全メッセージはJSON形式：  
```json  
{  
  "event": "イベント名",  
  "path": "デバイスパス_Base64エンコード",  
  "payload": {}  
}  
```  

**主要クライアントコマンド：**  
| イベント | ペイロード | 説明 |  
|----------|------------|------|  
| `setBrightness` | `{ "brightness": 0-100 }` | 画面輝度設定 |  
| `setKeyImg` | `{ "keyIndex": N, "imagePath": "パス" }` | キーアイコン設定 |  
| `refresh` | `{}` | 画面更新 |  
```
