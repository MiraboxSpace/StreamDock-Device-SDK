<p align="center">  
  <img src="../_Images/logo.png" height="128">  
  <h1 align="center">StreamDock Device SDK</h1>  
</p>  
  
<p align="center">  
  <a aria-label="Entrar na Comunidade Discord" href="https://discord.gg/WvCkKRGavX">  
    <img alt="" src="https://img.shields.io/badge/Entrar%20no%20Discord-5865F2.svg?style=for-the-badge&logo=Discord&logoColor=white">  
  </a>  
</p>  
  
## Índice  
  
- [Visão Geral](#visão-geral)  
- [Dispositivos Suportados](#dispositivos-suportados)  
- [Python SDK](#python-sdk)  
  - [Plataformas Suportadas](#plataformas-suportadas-python)  
  - [Instalação](#instalação-python)  
  - [Início Rápido](#início-rápido-python)  
  - [Objetos da API Python](#objetos-da-api-python)  
- [C++ SDK](#c-sdk)  
  - [Instalação Linux](#instalação-linux-c)  
  - [Instalação Windows](#instalação-windows-c)  
  - [Instalação macOS](#instalação-macos-c)  
  - [Objetos da API C++](#objetos-da-api-c)  
- [WebSocket SDK](#websocket-sdk)  
  - [Instalação](#instalação-websocket)  
  - [Eventos da API WebSocket](#eventos-da-api-websocket)  
- [Serviço Systemd (K1 PRO)](#serviço-systemd-k1-pro)  
- [Solução de Problemas](#solução-de-problemas)  
- [Licença](#licença)  
  
---  
  
## Visão Geral  
  
O **StreamDock Device SDK** permite controlar dispositivos StreamDock diretamente via código. O projeto oferece três SDKs independentes:  
  
| SDK | Linguagem | Plataformas |  
|-----|-----------|-------------|  
| Python SDK | Python 3.10+ | Linux, Windows, macOS |  
| C++ SDK | C++17 | Linux, Windows, macOS |  
| WebSocket SDK | Qualquer (protocolo WebSocket) | Windows |  
  
Documentação oficial completa: [https://creator.key123.vip/en/guide/get-started.html](https://creator.key123.vip/en/guide/get-started.html)  
  
---  
  
## Dispositivos Suportados  
  
| Modelo | Série |  
|--------|-------|  
| StreamDock 293 | 293, 293s, 293V3, 293sV3 |  
| StreamDock N | N1, N3, N4, N4 Pro |  
| StreamDock XL | XL |  
| StreamDock M | M3, M18 |  
| K1 Pro | K1 Pro |  
  
---  
  
## Python SDK  
  
### Plataformas Suportadas (Python)  
  
| Plataforma | Status | Descrição |  
|------------|--------|-----------|  
| Linux (x64, arm64) | ✅ Suportado | Ubuntu 20.04+, recomenda-se pyudev para monitoramento |  
| Windows (x64) | ✅ Suportado | Suporta WMI e modo polling |  
| macOS (x64, arm64) | ✅ Suportado | Usa modo polling para monitoramento |  
  
### Instalação (Python)  
  
#### Linux  
  
> Ambiente recomendado: Ubuntu 20.04 + Python 3.10 ou superior  
  
**1. Dependências Python:**  
  
```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

**2. Dependências do sistema:**  
```bash  
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0  
```  
⚠️ **Importante:** O pacote `libusb-1.0-0-dev` deve ser instalado antes do `libhidapi-libusb0`. É necessário executar com `sudo`.  

#### Windows  
  
> Ambiente recomendado: Windows 10/11 + Python 3.10  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
```  

Drivers geralmente são instalados automaticamente. Se houver problemas, instale:  
- [Microsoft Visual C++ Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe)  
- [HIDAPI Windows Driver](https://github.com/libusb/hidapi)  

#### macOS  
  
> Ambiente recomendado: macOS 11 + Python 3.10 ou superior  

```bash  
cd Python-SDK  
pip install -r requirements.txt  
brew install hidapi  
```  

### Início Rápido (Python)  

```python
from StreamDock.DeviceManager import DeviceManager  
from StreamDock.Devices.StreamDockN4Pro import StreamDockN4Pro  
import threading  
import time  

def key_callback(device, event):  
    from StreamDock.InputTypes import EventType  
    if event.event_type == EventType.BUTTON:  
        action = "pressionado" if event.state == 1 else "solto"  
        print(f"Tecla {event.key.value} foi {action}", flush=True)  

def main():  
    time.sleep(0.5)  
    manager = DeviceManager()  
    dispositivos = manager.enumerate()  

    if not dispositivos:  
        print("Nenhum dispositivo StreamDock encontrado")  
        return  

    # Escutar eventos de conexão/desconexão  
    t = threading.Thread(target=manager.listen)  
    t.daemon = True  
    t.start()  

    print(f"Encontrado(s) {len(dispositivos)} dispositivo(s).\n")  

    for device in dispositivos:  
        device.open()  
        device.init()  
        print(f"path: {device.path}, firmware: {device.firmware_version}, serial: {device.serial_number}")  

        # Definir imagem de fundo  
        device.set_touchscreen_image("img/backgroud_test.png")  
        device.refresh()  
        time.sleep(2)  

        # Definir imagem de tecla  
        device.set_key_image(1, "img/button_test.jpg")  
        device.refresh()  

        # Controle de LED (apenas N4 Pro)  
        if isinstance(device, StreamDockN4Pro):  
            device.set_led_brightness(100)  
            device.set_led_color(0, 0, 255)  

        # Registrar callback de eventos  
        device.set_key_callback(key_callback)  

    print("Programa em execução, pressione Ctrl+C para sair...")  
    try:  
        while True:  
            time.sleep(0.1)  
    except KeyboardInterrupt:  
        print("\nEncerrando dispositivos...")  
    finally:  
        for device in dispositivos:  
            device.close()  
        print("Programa encerrado")  

if __name__ == "__main__":  
    main()  
```

### Objetos da API Python  

**DeviceManager** (`Python-SDK/src/StreamDock/DeviceManager.py`)  
Gerenciador central de dispositivos. Responsável por enumerar, conectar e monitorar hot-plug.  

| Método | Descrição |  
|--------|-----------|  
| `enumerate() -> list` | Enumera todos os dispositivos StreamDock conectados. Retorna lista de objetos `StreamDock`. |  
| `listen()` | Inicia monitoramento de hot-plug (conexão/desconexão). Deve ser executado em uma thread separada. |  

**StreamDock** (classe base abstrata) (`Python-SDK/src/StreamDock/Devices/StreamDock.py`)  
Classe base para todos os dispositivos. Não deve ser instanciada diretamente.  

| Método | Descrição |  
|--------|-----------|  
| `open()` | Abre a conexão com o dispositivo. Inicia thread de leitura e heartbeat. |  
| `init()` | Inicializa o dispositivo: acorda tela, define brilho, limpa ícones, obtém firmware. |  
| `close()` | Fecha o dispositivo e libera todos os recursos. |  
| `set_key_image(key, path)` | Define a imagem de uma tecla específica (índice a partir de 1). |  
| `set_touchscreen_image(path)` | Define a imagem de fundo da tela. |  
| `set_brightness(percent)` | Define o brilho da tela (0–100). |  
| `refresh()` | Atualiza o display do dispositivo. |  
| `clearIcon(index)` | Limpa o ícone de uma tecla específica. |  
| `clearAllIcon()` | Limpa todos os ícones de teclas. |  
| `wakeScreen()` | Acorda a tela do dispositivo. |  
| `disconnected()` | Envia comando de desconexão. |  
| `set_key_callback(callback)` | Registra callback para eventos de entrada. Assinatura: `callback(device, event: InputEvent)`. |  

**Classes de Dispositivos Específicos**  
Cada modelo herda de `StreamDock` e implementa os métodos abstratos:  

| Classe | Arquivo | Modelo |  
|--------|---------|--------|  
| `StreamDock293` | `Devices/StreamDock293.py` | 293 |  
| `StreamDock293V3` | `Devices/StreamDock293V3.py` | 293V3 |  
| `StreamDockN4Pro` | `Devices/StreamDockN4Pro.py` | N4 Pro |  
| `K1Pro` | `Devices/K1Pro.py` | K1 Pro |  

---  

## C++ SDK  
  
### Instalação Linux (C++)  

> Ambiente recomendado: Ubuntu 20.04+, CMake 3.16+, C++17  

1. **Dependências do sistema:**  
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

4. **Executar:**  
```bash  
sudo ./bin/main  
```  

**Para evitar `sudo`:** Copie `99-streamdock.rules` para `/etc/udev/rules.d/` e execute:  
```bash  
sudo udevadm control --reload-rules  
sudo udevadm trigger  
```  

### Instalação Windows (C++)  

> Ambiente recomendado: Visual Studio 2022, C++17  

Bibliotecas pré-compiladas em `bin/`:  
- `opencv_core4120.dll`  
- `opencv_imgcodecs4120.dll`  
- `opencv_imgproc4120.dll`  
- `hidapi.dll`  

**Compilar:**  
```bash  
cd CPP-SDK  
build.bat  
```  

Ou abra no Visual Studio 2022 e compile com `Ctrl+B`.  

### Instalação macOS (C++)  

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

### Instalação (WebSocket)  

Execute `WebsocketSDK.exe` (porta padrão: 9002):  
```bash  
WebsocketSDK.exe  
# ou com porta personalizada:  
WebsocketSDK.exe -port 9009  
```  

Conecte via WebSocket: `ws://127.0.0.1:9002`  

**Dependências para teste Python:**  
```bash  
pip install websockets  
```  

### Eventos da API WebSocket  

Todas as mensagens usam formato JSON:  
```json  
{  
  "event": "nome_do_evento",  
  "path": "caminho_do_dispositivo_em_Base64",  
  "payload": {}  
}  
```  

**Principais comandos do cliente:**  
| Evento | Payload | Descrição |  
|--------|---------|-----------|  
| `setBrightness` | `{ "brightness": 0-100 }` | Define brilho da tela |  
| `setKeyImg` | `{ "keyIndex": N, "imagePath": "caminho" }` | Define ícone de tecla |  
| `refresh` | `{}` | Atualiza a tela |  

---  

