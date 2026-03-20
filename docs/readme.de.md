<p align="center">  
  <img src="../_Images/logo.png" height="128">  
  <h1 align="center">StreamDock Device SDK</h1>  
</p>  
  
<p align="center">  
  <a aria-label="Discord-Community beitreten" href="https://discord.gg/WvCkKRGavX">  
    <img alt="" src="https://img.shields.io/badge/Discord%20beitreten-5865F2.svg?style=for-the-badge&logo=Discord&logoColor=white">  
  </a>  
</p>  
  
## Inhaltsverzeichnis  
  
- [Übersicht](#übersicht)  
- [Unterstützte Geräte](#unterstützte-geräte)  
- [Python SDK](#python-sdk)  
  - [Unterstützte Plattformen](#unterstützte-plattformen-python)  
  - [Installation](#installation-python)  
  - [Schnellstart](#schnellstart-python)  
  - [Python API-Objekte](#python-api-objekte)  
- [C++ SDK](#c-sdk)  
  - [Linux Installation](#installation-linux-c)  
  - [Windows Installation](#installation-windows-c)  
  - [macOS Installation](#installation-macos-c)  
  - [C++ API-Objekte](#c-api-objekte)  
- [WebSocket SDK](#websocket-sdk)  
  - [Installation](#installation-websocket)  
  - [WebSocket API-Ereignisse](#websocket-api-ereignisse)  
- [Fehlerbehebung](#fehlerbehebung)  
- [Lizenz](#lizenz)  
  
---  
  
## Übersicht  
  
Das **StreamDock Device SDK** ermöglicht die direkte Steuerung von StreamDock-Geräten über Code. Das Projekt bietet drei unabhängige SDKs:  
  
| SDK | Sprache | Plattformen |  
|-----|---------|-------------|  
| Python SDK | Python 3.10+ | Linux, Windows, macOS |  
| C++ SDK | C++17 | Linux, Windows, macOS |  
| WebSocket SDK | Beliebig (WebSocket-Protokoll) | Windows |  
  
Vollständige offizielle Dokumentation: [https://creator.key123.vip/en/guide/get-started.html](https://creator.key123.vip/en/guide/get-started.html)  
  
---  
  
## Unterstützte Geräte  
  
| Modell | Serie |  
|--------|-------|  
| StreamDock 293 | 293, 293s, 293V3, 293sV3 |  
| StreamDock N | N1, N3, N4, N4 Pro |  
| StreamDock XL | XL |  
| StreamDock M | M3, M18 |  
| K1 Pro | K1 Pro |  
  
---  
  
## Python SDK  
  
### Unterstützte Plattformen (Python)  
  
| Plattform | Status | Beschreibung |  
|-----------|--------|--------------|  
| Linux (x64, arm64) | ✅ Unterstützt | Ubuntu 20.04+, pyudev für Überwachung empfohlen |  
| Windows (x64) | ✅ Unterstützt | Unterstützt WMI und Polling-Modus |  
| macOS (x64, arm64) | ✅ Unterstützt | Verwendet Polling-Modus für Überwachung |  
  
### Installation (Python)  
  
#### Linux  
  
> Empfohlene Umgebung: Ubuntu 20.04 + Python 3.10 oder höher  
  
**1. Python-Abhängigkeiten:**  
  
```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

**2. Systemabhängigkeiten:**  
```bash  
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0  
```  
⚠️ **Wichtig:** Das Paket `libusb-1.0-0-dev` muss vor `libhidapi-libusb0` installiert werden. Erfordert `sudo`.  

#### Windows  
  
> Empfohlene Umgebung: Windows 10/11 + Python 3.10  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

Treiber werden normalerweise automatisch installiert. Bei Problemen installieren:  
- [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)  
- [HIDAPI Windows Driver](https://github.com/libusb/hidapi)  

#### macOS  
  
> Empfohlene Umgebung: macOS 11 + Python 3.10 oder höher  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
brew install hidapi  
```  

### Schnellstart (Python)  

```python
from StreamDock.DeviceManager import DeviceManager  
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro  
import threading  
import time  

def key_callback(device, event):  
    from StreamDock.InputTypes import EventType  
    if event.event_type == EventType.BUTTON:  
        action = "gedrückt" if event.state == 1 else "losgelassen"  
        print(f"Taste {event.key.value} wurde {action}", flush=True)  

def main():  
    time.sleep(0.5)  
    manager = DeviceManager()  
    devices = manager.enumerate()  

    if not devices:  
        print("Keine StreamDock-Geräte gefunden")  
        return  

    # Verbindung/Trennung-Ereignisse überwachen  
    t = threading.Thread(target=manager.listen)  
    t.daemon = True  
    t.start()  

    print(f"{len(devices)} Gerät(e) gefunden.\n")  

    for device in devices:  
        device.open()  
        device.init()  
        print(f"Pfad: {device.path}, Firmware: {device.firmware_version}, Seriennummer: {device.serial_number}")  

        # Hintergrundbild setzen  
        device.set_touchscreen_image("img/backgroud_test.png")  
        device.refresh()  
        time.sleep(2)  

        # Tastenbild setzen  
        device.set_key_image(1, "img/button_test.jpg")  
        device.refresh()  

        # LED-Steuerung (nur N4 Pro)  
        if isinstance(device, StreamDockN4Pro):  
            device.set_led_brightness(100)  
            device.set_led_color(0, 0, 255)  

        # Ereignis-Callback registrieren  
        device.set_key_callback(key_callback)  

    print("Programm läuft, Ctrl+C zum Beenden...")  
    try:  
        while True:  
            time.sleep(0.1)  
    except KeyboardInterrupt:  
        print("\nGeräte werden geschlossen...")  
    finally:  
        for device in devices:  
            device.close()  
        print("Programm beendet")  

if __name__ == "__main__":  
    main()  
```

### Python API-Objekte  

**DeviceManager** (`Python-SDK/src/StreamDock/DeviceManager.py`)  
Zentrale Geräteverwaltung. Zuständig für Aufzählung, Verbindung und Hot-Plug-Überwachung.  

| Methode | Beschreibung |  
|---------|--------------|  
| `enumerate() -> list` | Listet alle verbundenen StreamDock-Geräte auf. Gibt Liste von `StreamDock`-Objekten zurück. |  
| `listen()` | Startet Hot-Plug-Überwachung (Verbindung/Trennung). Muss in separatem Thread ausgeführt werden. |  

**StreamDock** (Basis-Abstractklasse) (`Python-SDK/src/StreamDock/Devices/StreamDock.py`)  
Basis-Klasse für alle Geräte. Nicht direkt instanziieren.  

| Methode | Beschreibung |  
|---------|--------------|  
| `open()` | Öffnet Geräteverbindung. Startet Lese-Thread und Heartbeat. |  
| `init()` | Initialisiert Gerät: Bildschirm aktivieren, Helligkeit setzen, Icons löschen, Firmware abrufen. |  
| `close()` | Schließt Gerät und gibt alle Ressourcen frei. |  
| `set_key_image(key, path)` | Bild für spezifische Taste setzen (Index beginnt bei 1). |  
| `set_touchscreen_image(path)` | Touchscreen-Hintergrundbild setzen. |  
| `set_brightness(percent)` | Bildschirmhelligkeit setzen (0–100). |  
| `refresh()` | Geräte-Display aktualisieren. |  

**Spezifische Geräteklassen**  
Jedes Modell erbt von `StreamDock` und implementiert abstrakte Methoden:  

| Klasse | Datei | Modell |  
|--------|-------|--------|  
| `StreamDock293` | `Devices/StreamDock293.py` | 293 |  
| `StreamDock293V3` | `Devices/StreamDock293V3.py` | 293V3 |  
| `StreamDockN4Pro` | `Devices/StreamDockN4Pro.py` | N4 Pro |  
| `K1Pro` | `Devices/K1Pro.py` | K1 Pro |  

---  

## C++ SDK  
  
### Linux Installation (C++)  

> Empfohlene Umgebung: Ubuntu 20.04+, CMake 3.16+, C++17  

1. **Systemabhängigkeiten:**  
```bash  
sudo apt update  
sudo apt install build-essential cmake  
sudo apt install -y libudev-dev libusb-1.0-0-dev libgif-dev  
```  

2. **OpenCV:**  
```bash  
sudo apt install libopencv-dev  
```  

3. **Kompilieren:**  
```bash  
cd CPP-SDK  
chmod +x build.sh  
./build.sh  
```  

4. **Ausführen:**  
```bash  
sudo ./bin/main  
```  

**Um `sudo` zu vermeiden:** Kopieren Sie `99-streamdock.rules` nach `/etc/udev/rules.d/` und führen Sie aus:  
```bash  
sudo udevadm control --reload-rules  
sudo udevadm trigger  
```  

### Windows Installation (C++)  

> Empfohlene Umgebung: Visual Studio 2022, C++17  

Vorkompilierte Bibliotheken im `bin/`-Verzeichnis:  
- `opencv_core4120.dll`  
- `opencv_imgcodecs4120.dll`  
- `opencv_imgproc4120.dll`  
- `hidapi.dll`  

**Kompilieren:**  
```bash  
cd CPP-SDK  
build.bat  
```  

Oder in Visual Studio 2022 öffnen und mit `Ctrl+B` kompilieren.  

### macOS Installation (C++)  

1. **OpenCV vorbereiten:**  
```bash  
cd CPP-SDK/ImgProcesser/third_party  
bash link_opencv_symlinks.sh  
```  

2. **Kompilieren:**  
```bash  
cd CPP-SDK  
chmod +x build.sh  
./build.sh  
```  

---  

## WebSocket SDK  

### Installation (WebSocket)  

Führen Sie `WebsocketSDK.exe` aus (Standardport: 9002):  
```bash  
WebsocketSDK.exe  
# oder mit benutzerdefiniertem Port:  
WebsocketSDK.exe -port 9009  
```  

WebSocket verbinden: `ws://127.0.0.1:9002`  

**Python-Testabhängigkeiten:**  
```bash  
pip install websockets  
```  

### WebSocket API-Ereignisse  

Alle Nachrichten verwenden JSON-Format:  
```json  
{  
  "event": "event_name",  
  "path": "device_path_in_Base64",  
  "payload": {}  
}  
```  

**Haupt-Client-Befehle:**  
| Ereignis | Payload | Beschreibung |  
|----------|---------|--------------|  
| `setBrightness` | `{ "brightness": 0-100 }` | Bildschirmhelligkeit setzen |  
| `setKeyImg` | `{ "keyIndex": N, "imagePath": "Pfad" }` | Tasten-Symbol setzen |  
| `refresh` | `{}` | Bildschirm aktualisieren |  
```
