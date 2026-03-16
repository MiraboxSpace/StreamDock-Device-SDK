# k1pro_linux.py  
#  
######################################################################################  
#  
# PT_BR:  
# Script principal para o K1 PRO no Linux.  
#  
# Este script é o ponto de entrada que:  
#   1. Busca dispositivos K1 PRO conectados via USB  
#   2. Abre e inicializa cada K1 PRO encontrado  
#   3. Registra um callback que recebe TODOS os eventos de input (teclas e knobs)  
#   4. Despacha eventos de knobs para os handlers globais (knob_handlers.py)  
#   5. Despacha eventos de teclas para os handlers por página (key_handlers.py)  
#   6. Mantém o processo rodando até Ctrl+C  
#   7. Ao encerrar, fecha todos os dispositivos de forma segura  
#  
# Dependências:  
#   - StreamDock SDK (Python-SDK/src/StreamDock/)  
#   - handlers/knob_handlers.py (handlers dos knobs)  
#   - handlers/key_handlers.py (handlers das teclas por página)  
#   - pactl (sudo apt install pulseaudio-utils)  
#   - xdotool (sudo apt install xdotool)  
#   - Regras udev instaladas (99-streamdock.rules)  
#  
# Como rodar:  
#   cd Python-SDK/src  
#   source ../../.venv/bin/activate  
#   python k1pro_linux.py  
#  
######################################################################################  
#  
# EN_US:  
# Main script for the K1 PRO on Linux.  
#  
# This script is the entry point that:  
#   1. Searches for K1 PRO devices connected via USB  
#   2. Opens and initializes each K1 PRO found  
#   3. Registers a callback that receives ALL input events (keys and knobs)  
#   4. Dispatches knob events to the global handlers (knob_handlers.py)  
#   5. Dispatches key events to the per-page handlers (key_handlers.py)  
#   6. Keeps the process running until Ctrl+C  
#   7. On shutdown, closes all devices safely  
#  
# Dependencies:  
#   - StreamDock SDK (Python-SDK/src/StreamDock/)  
#   - handlers/knob_handlers.py (knob handlers)  
#   - handlers/key_handlers.py (key handlers per page)  
#   - pactl (sudo apt install pulseaudio-utils)  
#   - xdotool (sudo apt install xdotool)  
#   - udev rules installed (99-streamdock.rules)  
#  
# How to run:  
#   cd Python-SDK/src  
#   source ../../.venv/bin/activate  
#   python k1pro_linux.py  
#  
######################################################################################  
  
from StreamDock.DeviceManager import DeviceManager  
from StreamDock.Devices.K1Pro import K1Pro  
from StreamDock.InputTypes import EventType, KnobId  
from handlers.knob_handlers import handle_knob_1, handle_knob_2, handle_knob_3  
from handlers.key_handlers import handle_key, apply_page_images, get_current_page_name, get_page_names
import threading  
import time  
  
  
######################################################################################  
#  
# PT_BR:  
# Dicionário que mapeia cada KnobId para sua função handler.  
# Isso permite despachar eventos de knob de forma limpa, sem if/elif encadeados.  
# Para trocar a função de um knob, basta apontar para outro handler.  
#  
# Exemplo: se quiser que o Knob 1 controle brilho, crie handle_knob_1_brightness()  
# no knob_handlers.py e troque aqui:  
#   KnobId.KNOB_1: handle_knob_1_brightness,  
#  
######################################################################################  
#  
# EN_US:  
# Dictionary that maps each KnobId to its handler function.  
# This allows dispatching knob events cleanly, without chained if/elif.  
# To change a knob's function, just point to a different handler.  
#  
# Example: if you want Knob 1 to control brightness, create  
# handle_knob_1_brightness() in knob_handlers.py and change here:  
#   KnobId.KNOB_1: handle_knob_1_brightness,  
#  
######################################################################################  
  
KNOB_HANDLERS = {  
    KnobId.KNOB_1: handle_knob_1,  
    KnobId.KNOB_2: handle_knob_2,  
    KnobId.KNOB_3: handle_knob_3,  
}  
  
  
######################################################################################  
#  
# PT_BR:  
# Callback principal que recebe TODOS os eventos de input do K1 PRO.  
# É chamado pela thread de leitura do SDK toda vez que o dispositivo  
# envia dados (tecla pressionada/solta, knob girado/pressionado).  
#  
# Fluxo:  
#   1. Se for evento de knob (KNOB_ROTATE ou KNOB_PRESS):  
#      -> Despacha para o handler global via KNOB_HANDLERS dict  
#      -> Retorna imediatamente (knobs são globais, não dependem de página)  
#   2. Se for evento de tecla (BUTTON):  
#      -> Despacha para handle_key() do key_handlers.py  
#      -> O key_handlers resolve qual ação executar com base na página ativa  
#  
# IMPORTANTE: Este callback roda na thread de leitura do SDK.  
# Se bloquear aqui, o SDK não consegue ler novos eventos do dispositivo.  
# Por isso, ações pesadas devem rodar em threads separadas  
# (os handlers já fazem isso quando necessário).  
#  
######################################################################################  
#  
# EN_US:  
# Main callback that receives ALL input events from the K1 PRO.  
# Called by the SDK's read thread every time the device sends data  
# (key pressed/released, knob rotated/pressed).  
#  
# Flow:  
#   1. If it's a knob event (KNOB_ROTATE or KNOB_PRESS):  
#      -> Dispatches to the global handler via KNOB_HANDLERS dict  
#      -> Returns immediately (knobs are global, don't depend on page)  
#   2. If it's a key event (BUTTON):  
#      -> Dispatches to handle_key() from key_handlers.py  
#      -> key_handlers resolves which action to execute based on the active page  
#  
# IMPORTANT: This callback runs on the SDK's read thread.  
# If it blocks here, the SDK can't read new events from the device.  
# Therefore, heavy actions should run in separate threads  
# (the handlers already do this when necessary).  
#  
######################################################################################  
  
def key_callback(device, event):  
    try:  
        # ==================================================================  
        # PT_BR: KNOBS - HANDLERS GLOBAIS  
        #         Independentes de página. Sempre executam a mesma ação.  
        #         O dict KNOB_HANDLERS mapeia KnobId -> função handler.  
        #         getattr() é usado para extrair direction e state do evento  
        #         de forma segura (retorna None se o atributo não existir).  
        #  
        # EN_US: KNOBS - GLOBAL HANDLERS  
        #         Page-independent. Always execute the same action.  
        #         The KNOB_HANDLERS dict maps KnobId -> handler function.  
        #         getattr() is used to safely extract direction and state  
        #         from the event (returns None if the attribute doesn't exist).  
        # ==================================================================  
        if event.event_type in (EventType.KNOB_ROTATE, EventType.KNOB_PRESS):  
            handler = KNOB_HANDLERS.get(event.knob_id)  
            if handler:  
                handler(  
                    event_type=event.event_type,  
                    direction=getattr(event, "direction", None),  
                    state=getattr(event, "state", None),  
                )  
            return  
  
        # ==================================================================  
        # PT_BR: TECLAS 1-6 - LÓGICA POR PÁGINA  
        #         Despacha para o key_handlers.py que resolve qual ação  
        #         executar com base na página ativa (current_page).  
        #         O handle_key() já ignora eventos de release (state == 0)  
        #         internamente, então podemos passar tudo.  
        #  
        # EN_US: KEYS 1-6 - PER-PAGE LOGIC  
        #         Dispatches to key_handlers.py which resolves which action  
        #         to execute based on the active page (current_page).  
        #         handle_key() already ignores release events (state == 0)  
        #         internally, so we can pass everything through.  
        # ==================================================================  
        # === TECLAS: POR PÁGINA ===  
        if event.event_type == EventType.BUTTON:  
            print(f"DEBUG: key={event.key}, state={event.state}", flush=True)  # <-- adicione isso  
            handle_key(key=event.key, state=event.state)
  
    except Exception as e:  
        print(f"Callback error: {e}", flush=True)  
        import traceback  
        traceback.print_exc()  
  
  
######################################################################################  
#  
# PT_BR:  
# Função principal. Inicializa o SDK, busca dispositivos, configura callbacks  
# e mantém o processo rodando.  
#  
# Fluxo:  
#   1. Aguarda 0.5s para estabilização do USB (evita race conditions no Linux)  
#   2. Cria o DeviceManager e enumera dispositivos conectados  
#   3. Para cada K1 PRO encontrado:  
#      a. Abre a conexão USB (device.open())  
#      b. Inicializa o dispositivo (device.init())  
#      c. Registra o callback de eventos  
#      d. Configura iluminação do teclado  
#   4. Inicia thread de hotplug (detecta conexão/desconexão de dispositivos)  
#   5. Entra no loop principal (sleep 0.1s) até Ctrl+C  
#   6. No finally, fecha todos os dispositivos de forma segura  
#  
# EN_US:  
# Main function. Initializes the SDK, searches for devices, configures callbacks  
# and keeps the process running.  
#  
# Flow:  
#   1. Waits 0.5s for USB stabilization (avoids race conditions on Linux)  
#   2. Creates DeviceManager and enumerates connected devices  
#   3. For each K1 PRO found:  
#      a. Opens the USB connection (device.open())  
#      b. Initializes the device (device.init())  
#      c. Registers the event callback  
#      d. Configures keyboard lighting  
#   4. Starts hotplug thread (detects device connect/disconnect)  
#   5. Enters main loop (sleep 0.1s) until Ctrl+C  
#   6. In finally, closes all devices safely  
#  
######################################################################################  
  
def main():  
    # PT_BR: Aguarda 0.5s para estabilização do subsistema USB no Linux.  
    #         Sem isso, enumerate() pode não encontrar dispositivos recém-conectados.  
    # EN_US: Waits 0.5s for the Linux USB subsystem to stabilize.  
    #         Without this, enumerate() may miss recently connected devices.  
    time.sleep(0.5)  
  
    # PT_BR: DeviceManager é a classe do SDK que gerencia a descoberta de dispositivos.  
    #         enumerate() retorna uma lista de objetos StreamDock (K1Pro, N4Mini, etc.)  
    # EN_US: DeviceManager is the SDK class that manages device discovery.  
    #         enumerate() returns a list of StreamDock objects (K1Pro, N4Mini, etc.)  
    manager = DeviceManager()  
    devices = manager.enumerate()  
  
    if not devices:  
        print("========================================")  
        print("  Nenhum dispositivo encontrado!")  
        print("  No device found!")  
        print("========================================")  
        print("")  
        print("  Verifique / Check:")  
        print("    lsusb | grep 6603")  
        print("    ls -la /dev/hidraw*")  
        print("")  
        return  
  
    print(f"Encontrado(s) {len(devices)} dispositivo(s) / Found {len(devices)} device(s)")  
  
    k1pro_devices = []  
  
    for device in devices:  
        # PT_BR: Filtra apenas dispositivos K1 PRO.  
        #         O SDK pode encontrar outros modelos (N4Mini, 293S, etc.)  
        #         que não são o que queremos controlar aqui.  
        # EN_US: Filters only K1 PRO devices.  
        #         The SDK may find other models (N4Mini, 293S, etc.)  
        #         that are not what we want to control here.  
        if not isinstance(device, K1Pro):  
            print(f"Ignorando / Skipping non-K1Pro: {type(device).__name__}")  
            continue  
  
        print("K1 PRO encontrado! / K1 PRO found!")  
  
        try:  
            # PT_BR: open() faz:  
            #         1. Abre a conexão USB via libtransport.so  
            #         2. Inicia a thread de leitura (recebe dados do dispositivo)  
            #         3. Inicia a thread de heartbeat (mantém conexão viva)  
            # EN_US: open() does:  
            #         1. Opens USB connection via libtransport.so  
            #         2. Starts the read thread (receives data from device)  
            #         3. Starts the heartbeat thread (keeps connection alive)  
            device.open()  
  
            # PT_BR: init() faz:  
            #         1. Configura o dispositivo (set_device)  
            #         2. Acorda a tela (wakeScreen)  
            #         3. Define brilho em 100% (set_brightness)  
            #         4. Limpa todos os ícones (clearAllIcon)  
            # EN_US: init() does:  
            #         1. Configures the device (set_device)  
            #         2. Wakes the screen (wakeScreen)  
            #         3. Sets brightness to 100% (set_brightness)  
            #         4. Clears all icons (clearAllIcon)  
            device.init()  
            apply_page_images(device)  # Aplica as imagens iniciais para a página atual (página 1 por padrão)
        except Exception as e:  
            print(f"Falha ao abrir / Failed to open: {e}")  
            import traceback  
            traceback.print_exc()  
            continue  
  
        print(f"  path: {device.path}")  
        print(f"  firmware: {device.firmware_version}")  
        print(f"  serial: {device.serial_number}")  
  
        # PT_BR: Registra o callback. A partir daqui, toda vez que o usuário  
        #         pressionar uma tecla ou girar um knob, key_callback() será chamado.  
        # EN_US: Registers the callback. From this point on, every time the user  
        #         presses a key or rotates a knob, key_callback() will be called.  
        device.set_key_callback(key_callback)  
  
        # PT_BR: Configuração visual do teclado mecânico (backlight das teclas físicas).  
        #         Isso NÃO afeta as telas LCD das 6 teclas virtuais.  
        #         - brightness: 0-6 (0 = apagado, 6 = máximo)  
        #         - lighting_effects: 0 = estático (cor fixa)  
        #         - rgb_backlight: cor RGB (0-255 cada canal)  
        # EN_US: Mechanical keyboard visual configuration (physical key backlight).  
        #         This does NOT affect the LCD screens of the 6 virtual keys.  
        #         - brightness: 0-6 (0 = off, 6 = maximum)  
        #         - lighting_effects: 0 = static (fixed color)  
        #         - rgb_backlight: RGB color (0-255 each channel)  
        device.set_keyboard_backlight_brightness(6)  
        device.set_keyboard_lighting_effects(0)  
        device.set_keyboard_rgb_backlight(0, 255, 0)  
  
        k1pro_devices.append(device)  
  
    if not k1pro_devices:  
        print("Nenhum K1 PRO encontrado entre os dispositivos. / No K1 PRO found among devices.")  
        return  
  
    # PT_BR: Inicia thread de hotplug em background.  
    #         Detecta quando dispositivos são conectados/desconectados via USB.  
    #         daemon=True faz a thread morrer automaticamente quando o processo principal encerra.  
    # EN_US: Starts hotplug thread in background.  
    #         Detects when devices are connected/disconnected via USB.  
    #         daemon=True makes the thread die automatically when the main process exits.  
    listen_thread = threading.Thread(target=manager.listen)  
    listen_thread.daemon = True  
    listen_thread.start()  
  
    print("")  
    print("========================================")  
    print("  K1 PRO Linux Config - Ativo / Active")  
    print("========================================")  
    print("")  
    print("  Knob 2: Microfone / Microphone")  
    print("    Girar / Rotate  = volume +/-")  
    print("    Pressionar / Press = mute/unmute")  
    print("")  
    print("  Knob 3: Alto-falante / Speaker")  
    print("    Girar / Rotate  = volume +/-")  
    print("    Pressionar / Press = mute/unmute")  
    print("")  
    print("  Knob 1: (sem ação / no action)")  
    print("")  
    print(f"  Página ativa / Active page: '{get_current_page_name()}'")  
    print(f"  Páginas disponíveis / Available pages: {get_page_names()}")  
    print("")  
    print("  Teclas / Keys 1-6: INSERT, DELETE, HOME, END, PGUP, PGDN")  
    print("")  
    print("  Ctrl+C para sair / to exit")  
    print("")  
  
    # PT_BR: Loop principal. Mantém o processo vivo enquanto as threads de  
    #         leitura e heartbeat rodam em background.  
    #         sleep(0.1) evita busy-waiting (uso excessivo de CPU).  
    # EN_US: Main loop. Keeps the process alive while the read and heartbeat  
    #         threads run in the background.  
    #         sleep(0.1) avoids busy-waiting (excessive CPU usage).  
    try:  
        while True:  
            time.sleep(0.1)  
    except KeyboardInterrupt:  
        print("\nEncerrando... / Shutting down...")  
  
    # PT_BR: CLEANUP - Fecha todos os dispositivos de forma segura.  
    #         IMPORTANTE: Deve ser feito na ordem reversa (último aberto = primeiro fechado)  
    #         para evitar Segmentation Fault no libusb.  
    #  
    #         Passos:  
    #           1. Remove o callback (evita que callbacks sejam chamados durante shutdown)  
    #           2. Aguarda 0.1s (dá tempo para a thread de leitura sair do loop)  
    #           3. Fecha o dispositivo (libera recursos USB)  
    #  
    # EN_US: CLEANUP - Closes all devices safely.  
    #         IMPORTANT: Must be done in reverse order (last opened = first closed)  
    #         to avoid Segmentation Fault in libusb.  
    #  
    #         Steps:  
    #           1. Remove the callback (prevents callbacks during shutdown)  
    #           2. Wait 0.1s (gives the read thread time to exit its loop)  
    #           3. Close the device (releases USB resources)  
    finally:  
        for device in reversed(k1pro_devices):  
            try:  
                device.set_key_callback(None)  
                time.sleep(0.1)  
                device.close()  
                print(f"Dispositivo fechado / Device closed: {device.path}")  
            except Exception as e:  
                print(f"Erro ao fechar / Error closing: {e}")  
  
        # PT_BR: Aguarda 0.2s final para garantir que todos os recursos C foram liberados.  
        #         A libtransport.so é uma biblioteca C que gerencia memória manualmente.  
        # EN_US: Final 0.2s wait to ensure all C resources have been released.  
        #         libtransport.so is a C library that manages memory manually.  
        time.sleep(0.2)  
        print("Finalizado. / Done.")  
  
  
######################################################################################  
#  
# PT_BR:  
# Ponto de entrada do script.  
# O try/except externo captura exceções não tratadas que escapam do main(),  
# como erros de importação ou problemas com a libtransport.so.  
# Isso evita que o script morra silenciosamente sem mostrar o erro.  
#  
# EN_US:  
# Script entry point.  
# The outer try/except catches unhandled exceptions that escape main(),  
# such as import errors or issues with libtransport.so.  
# This prevents the script from dying silently without showing the error.  
#  
######################################################################################  
  
if __name__ == "__main__":  
    try:  
        main()  
    except SystemExit as e:  
        print(f"\nTerminado por SystemExit / Terminated by SystemExit: {e}")  
        import traceback  
        traceback.print_exc()  
    except Exception as e:  
        print(f"\nErro não tratado / Uncaught exception: {e}")  
        import traceback  
        traceback.print_exc()