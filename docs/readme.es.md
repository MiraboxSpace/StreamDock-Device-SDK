<p align="center">  
  <img src="../_Images/logo.png" height="128">  
  <h1 align="center">StreamDock Device SDK</h1>  
</p>  
  
<p align="center">  
  <a aria-label="Únete a la Comunidad Discord" href="https://discord.gg/WvCkKRGavX">  
    <img alt="" src="https://img.shields.io/badge/Únete%20al%20Discord-5865F2.svg?style=for-the-badge&logo=Discord&logoColor=white">  
  </a>  
</p>  
  
## Índice  
  
- [Visión General](#visión-general)  
- [Dispositivos Soportados](#dispositivos-soportados)  
- [Python SDK](#python-sdk)  
  - [Plataformas Soportadas](#plataformas-soportadas-python)  
  - [Instalación](#instalación-python)  
  - [Inicio Rápido](#inicio-rápido-python)  
  - [Objetos de la API Python](#objetos-de-la-api-python)  
- [C++ SDK](#c-sdk)  
  - [Instalación Linux](#instalación-linux-c)  
  - [Instalación Windows](#instalación-windows-c)  
  - [Instalación macOS](#instalación-macos-c)  
  - [Objetos de la API C++](#objetos-de-la-api-c)  
- [WebSocket SDK](#websocket-sdk)  
  - [Instalación](#instalación-websocket)  
  - [Eventos de la API WebSocket](#eventos-de-la-api-websocket)  
- [Solución de Problemas](#solución-de-problemas)  
- [Licencia](#licencia)  
  
---  
  
## Visión General  
  
El **StreamDock Device SDK** permite controlar dispositivos StreamDock directamente mediante código. El proyecto ofrece tres SDKs independientes:  
  
| SDK | Lenguaje | Plataformas |  
|-----|----------|-------------|  
| Python SDK | Python 3.10+ | Linux, Windows, macOS |  
| C++ SDK | C++17 | Linux, Windows, macOS |  
| WebSocket SDK | Cualquier (protocolo WebSocket) | Windows |  
  
Documentación oficial completa: [https://creator.key123.vip/en/guide/get-started.html](https://creator.key123.vip/en/guide/get-started.html)  
  
---  
  
## Dispositivos Soportados  
  
| Modelo | Serie |  
|--------|-------|  
| StreamDock 293 | 293, 293s, 293V3, 293sV3 |  
| StreamDock N | N1, N3, N4, N4 Pro |  
| StreamDock XL | XL |  
| StreamDock M | M3, M18 |  
| K1 Pro | K1 Pro |  
  
---  
  
## Python SDK  
  
### Plataformas Soportadas (Python)  
  
| Plataforma | Estado | Descripción |  
|------------|--------|-------------|  
| Linux (x64, arm64) | ✅ Soportado | Ubuntu 20.04+, se recomienda pyudev para monitoreo |  
| Windows (x64) | ✅ Soportado | Soporta WMI y modo polling |  
| macOS (x64, arm64) | ✅ Soportado | Usa modo polling para monitoreo |  
  
### Instalación (Python)  
  
#### Linux  
  
> Entorno recomendado: Ubuntu 20.04 + Python 3.10 o superior  
  
**1. Dependencias Python:**  
  
```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

**2. Dependencias del sistema:**  
```bash  
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0  
```  
⚠️ **Importante:** El paquete `libusb-1.0-0-dev` debe instalarse antes que `libhidapi-libusb0`. Es necesario ejecutar con `sudo`.  

#### Windows  
  
> Entorno recomendado: Windows 10/11 + Python 3.10  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

Los drivers generalmente se instalan automáticamente. Si hay problemas, instale:  
- [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)  
- [HIDAPI Windows Driver](https://github.com/libusb/hidapi)  

#### macOS  
  
> Entorno recomendado: macOS 11 + Python 3.10 o superior  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
brew install hidapi  
```  

### Inicio Rápido (Python)  

```python
from StreamDock.DeviceManager import DeviceManager  
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro  
import threading  
import time  

def key_callback(device, event):  
    from StreamDock.InputTypes import EventType  
    if event.event_type == EventType.BUTTON:  
        action = "presionado" if event.state == 1 else "soltado"  
        print(f"Tecla {event.key.value} fue {action}", flush=True)  

def main():  
    time.sleep(0.5)  
    manager = DeviceManager()  
    dispositivos = manager.enumerate()  

    if not dispositivos:  
        print("Ningún dispositivo StreamDock encontrado")  
        return  

    # Escuchar eventos de conexión/desconexión  
    t = threading.Thread(target=manager.listen)  
    t.daemon = True  
    t.start()  

    print(f"Encontrado(s) {len(dispositivos)} dispositivo(s).\n")  

    for device in dispositivos:  
        device.open()  
        device.init()  
        print(f"path: {device.path}, firmware: {device.firmware_version}, serial: {device.serial_number}")  

        # Definir imagen de fondo  
        device.set_touchscreen_image("img/backgroud_test.png")  
        device.refresh()  
        time.sleep(2)  

        # Definir imagen de tecla  
        device.set_key_image(1, "img/button_test.jpg")  
        device.refresh()  

        # Control de LED (solo N4 Pro)  
        if isinstance(device, StreamDockN4Pro):  
            device.set_led_brightness(100)  
            device.set_led_color(0, 0, 255)  

        # Registrar callback de eventos  
        device.set_key_callback(key_callback)  

    print("Programa en ejecución, presione Ctrl+C para salir...")  
    try:  
        while True:  
            time.sleep(0.1)  
    except KeyboardInterrupt:  
        print("\nCerrando dispositivos...")  
    finally:  
        for device in dispositivos:  
            device.close()  
        print("Programa finalizado")  

if __name__ == "__main__":  
    main()  
```

### Objetos de la API Python  

**DeviceManager** (`Python-SDK/src/StreamDock/DeviceManager.py`)  
Gestor central de dispositivos. Responsable de enumerar, conectar y monitorear hot-plug.  

| Método | Descripción |  
|--------|-------------|  
| `enumerate() -> list` | Enumera todos los dispositivos StreamDock conectados. Retorna lista de objetos `StreamDock`. |  
| `listen()` | Inicia monitoreo de hot-plug (conexión/desconexión). Debe ejecutarse en un thread separado. |  

**StreamDock** (clase base abstracta) (`Python-SDK/src/StreamDock/Devices/StreamDock.py`)  
Clase base para todos los dispositivos. No debe instanciarse directamente.  

| Método | Descripción |  
|--------|-------------|  
| `open()` | Abre la conexión con el dispositivo. Inicia thread de lectura y heartbeat. |  
| `init()` | Inicializa el dispositivo: despierta pantalla, define brillo, limpia íconos, obtiene firmware. |  
| `close()` | Cierra el dispositivo y libera todos los recursos. |  
| `set_key_image(key, path)` | Define la imagen de una tecla específica (índice desde 1). |  
| `set_touchscreen_image(path)` | Define la imagen de fondo de la pantalla. |  
| `set_brightness(percent)` | Define el brillo de la pantalla (0–100). |  
| `refresh()` | Actualiza la pantalla del dispositivo. |  

**Clases de Dispositivos Específicos**  
Cada modelo hereda de `StreamDock` e implementa los métodos abstractos:  

| Clase | Archivo | Modelo |  
|-------|---------|--------|  
| `StreamDock293` | `Devices/StreamDock293.py` | 293 |  
| `StreamDock293V3` | `Devices/StreamDock293V3.py` | 293V3 |  
| `StreamDockN4Pro` | `Devices/StreamDockN4Pro.py` | N4 Pro |  
| `K1Pro` | `Devices/K1Pro.py` | K1 Pro |  

---  

## C++ SDK  
  
### Instalación Linux (C++)  

> Entorno recomendado: Ubuntu 20.04+, CMake 3.16+, C++17  

1. **Dependencias del sistema:**  
```bash  
sudo apt update  
sudo apt install build-essential cmake  
sudo apt install -y libudev-dev libusb-1.0-0-dev libgif-dev  
```  

2. **OpenCV:**  
```bash  
sudo apt install libopencv-dev  
```  

3. **Compilar:**  
```bash  
cd CPP-SDK  
chmod +x build.sh  
./build.sh  
```  

4. **Ejecutar:**  
```bash  
sudo ./bin/main  
```  

**Para evitar `sudo`:** Copie `99-streamdock.rules` a `/etc/udev/rules.d/` y ejecute:  
```bash  
sudo udevadm control --reload-rules  
sudo udevadm trigger  
```  

### Instalación Windows (C++)  

> Entorno recomendado: Visual Studio 2022, C++17  

Bibliotecas precompiladas en `bin/`:  
- `opencv_core4120.dll`  
- `opencv_imgcodecs4120.dll`  
- `opencv_imgproc4120.dll`  
- `hidapi.dll`  

**Compilar:**  
```bash  
cd CPP-SDK  
build.bat  
```  

O abra en Visual Studio 2022 y compile con `Ctrl+B`.  

### Instalación macOS (C++)  

1. **Preparar OpenCV:**  
```bash  
cd CPP-SDK/ImgProcesser/third_party  
bash link_opencv_symlinks.sh  
```  

2. **Compilar:**  
```bash  
cd CPP-SDK  
chmod +x build.sh  
./build.sh  
```  

---  

## WebSocket SDK  

### Instalación (WebSocket)  

Ejecute `WebsocketSDK.exe` (puerto por defecto: 9002):  
```bash  
WebsocketSDK.exe  
# o con puerto personalizado:  
WebsocketSDK.exe -port 9009  
```  

Conecte vía WebSocket: `ws://127.0.0.1:9002`  

**Dependencias para prueba Python:**  
```bash  
pip install websockets  
```  

### Eventos de la API WebSocket  

Todas las mensajes usan formato JSON:  
```json  
{  
  "event": "nombre_del_evento",  
  "path": "ruta_del_dispositivo_en_Base64",  
  "payload": {}  
}  
```  

**Principales comandos del cliente:**  
| Evento | Payload | Descripción |  
|--------|---------|-------------|  
| `setBrightness` | `{ "brightness": 0-100 }` | Define brillo de la pantalla |  
| `setKeyImg` | `{ "keyIndex": N, "imagePath": "ruta" }` | Define ícono de tecla |  
| `refresh` | `{}` | Actualiza la pantalla |  
```

