# key_handlers.py  
#  
######################################################################################  
#  
# PT_BR:  
# Este módulo define os handlers para as 6 teclas do K1 PRO.  
# As teclas são organizadas por PÁGINAS (schemas), cada uma com seu mapeamento.  
# Diferente dos knobs (que são globais), as teclas mudam de comportamento  
# conforme a página ativa.  
#  
# Simulação de teclas:  
#   - X11:     usa xdotool (simula via protocolo X11)  
#   - Wayland: usa ydotool (simula via /dev/uinput, nível kernel)  
#  
# O ydotool usa keycodes do kernel Linux (linux/input-event-codes.h).  
# Cada tecla no schema tem um campo "keycode" com o código numérico correspondente.  
#  
# Dependências:  
#   - xdotool (sudo apt install xdotool) -- para X11  
#   - ydotool (sudo apt install ydotool) -- para Wayland  
#   - Permissão em /dev/uinput para ydotool sem sudo:  
#       sudo tee /etc/udev/rules.d/99-uinput.rules <<< 'KERNEL=="uinput", MODE="0666", GROUP="plugdev"'  
#       sudo udevadm control --reload-rules && sudo udevadm trigger  
#  
######################################################################################  
#  
# EN_US:  
# This module defines the handlers for the 6 keys on the K1 PRO.  
# Keys are organized by PAGES (schemas), each with its own mapping.  
# Unlike knobs (which are global), keys change behavior depending on  
# the active page.  
#  
# Key simulation:  
#   - X11:     uses xdotool (simulates via X11 protocol)  
#   - Wayland: uses ydotool (simulates via /dev/uinput, kernel level)  
#  
# ydotool uses Linux kernel keycodes (linux/input-event-codes.h).  
# Each key in the schema has a "keycode" field with the corresponding numeric code.  
#  
# Dependencies:  
#   - xdotool (sudo apt install xdotool) -- for X11  
#   - ydotool (sudo apt install ydotool) -- for Wayland  
#   - Permission on /dev/uinput for ydotool without sudo:  
#       sudo tee /etc/udev/rules.d/99-uinput.rules <<< 'KERNEL=="uinput", MODE="0666", GROUP="plugdev"'  
#       sudo udevadm control --reload-rules && sudo udevadm trigger  
#  
######################################################################################  
  
import os  
import subprocess  
from StreamDock.InputTypes import ButtonKey, EventType  
  
  
# =============================================================================  
# PT_BR: DETECÇÃO DE AMBIENTE  
#         Verifica se estamos em X11 ou Wayland para escolher a ferramenta  
#         de simulação de teclas correta.  
#  
# EN_US: ENVIRONMENT DETECTION  
#         Checks if we're on X11 or Wayland to choose the correct  
#         key simulation tool.  
# =============================================================================  
  
IS_X11 = (  
    os.environ.get("XDG_SESSION_TYPE", "").lower() == "x11"  
    or (  
        os.environ.get("XDG_SESSION_TYPE", "") == ""  
        and os.environ.get("DISPLAY", "") != ""  
    )  
)  
  
IS_WAYLAND = os.environ.get("XDG_SESSION_TYPE", "").lower() == "wayland"  
  
# PT_BR: Determina qual ferramenta de simulação usar.  
# EN_US: Determines which simulation tool to use.  
if IS_X11:  
    SIM_TOOL = "xdotool"  
elif IS_WAYLAND:  
    SIM_TOOL = "ydotool"  
else:  
    SIM_TOOL = None  

# =============================================================================  
#  
# PT_BR:  
# DETECÇÃO DE VERSÃO DO YDOTOOL  
#  
# O ydotool mudou a sintaxe entre versões:  
#   - Versão 0.x (legacy): ydotool key <keycode>  
#     O press e release são implícitos.  
#   - Versão 1.x+:         ydotool key <keycode>:1 <keycode>:0  
#     O press (1) e release (0) são explícitos.  
#  
# Detectamos a versão automaticamente ao carregar o módulo.  
# Se a detecção falhar, assumimos legacy (0.x) por ser mais compatível.  
#  
# EN_US:  
# YDOTOOL VERSION DETECTION  
#  
# ydotool changed its syntax between versions:  
#   - Version 0.x (legacy): ydotool key <keycode>  
#     Press and release are implicit.  
#   - Version 1.x+:         ydotool key <keycode>:1 <keycode>:0  
#     Press (1) and release (0) are explicit.  
#  
# We detect the version automatically when the module loads.  
# If detection fails, we assume legacy (0.x) as it's more compatible.  
#  
# =============================================================================  
YDOTOOL_LEGACY = False  
  
if SIM_TOOL == "ydotool":  
    try:  
        # PT_BR: Tenta obter a versão do ydotool.  
        #         --version pode retornar em stdout ou stderr dependendo da versão.  
        # EN_US: Tries to get the ydotool version.  
        #         --version may return in stdout or stderr depending on the version.  
        _version_result = subprocess.run(  
            ["ydotool", "--version"],  
            capture_output=True, text=True, timeout=5  
        )  
        _version_output = (_version_result.stdout + _version_result.stderr).strip().lower()  
  
        # PT_BR: Se contém "0." no início do número de versão, é legacy.  
        #         Exemplos: "0.1.8", "0.1.9-3build1"  
        # EN_US: If it contains "0." at the start of the version number, it's legacy.  
        #         Examples: "0.1.8", "0.1.9-3build1"  
        import re  
        if re.search(r'\b0\.\d', _version_output):  
            YDOTOOL_LEGACY = True  
    except Exception:  
        # PT_BR: Se não conseguir detectar, assume legacy (mais seguro).  
        # EN_US: If detection fails, assume legacy (safer).  
        YDOTOOL_LEGACY = True  
  
    print(f"[ydotool] version={'legacy (0.x)' if YDOTOOL_LEGACY else 'modern (1.x+)'}", flush=True)
  
print(f"[key_handlers] XDG_SESSION_TYPE={os.environ.get('XDG_SESSION_TYPE', '(not set)')}, "  
      f"IS_X11={IS_X11}, IS_WAYLAND={IS_WAYLAND}, SIM_TOOL={SIM_TOOL}", flush=True)  
  
  
# =============================================================================  
# PT_BR: DIRETÓRIO BASE PARA IMAGENS  
# EN_US: BASE DIRECTORY FOR IMAGES  
# =============================================================================  
  
_HANDLERS_DIR = os.path.dirname(os.path.abspath(__file__))  
_IMAGES_DIR = os.path.join(_HANDLERS_DIR, "images")  
  
  
# =============================================================================  
# PT_BR: FUNÇÃO AUXILIAR PARA EXECUTAR COMANDOS  
# EN_US: HELPER FUNCTION TO RUN COMMANDS  
# =============================================================================  
  
def _run_cmd(cmd):  
    """  
    PT_BR: Executa um comando externo de forma não-bloqueante.  
    EN_US: Runs an external command in a non-blocking way.  
    """  
    try:  
        subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)  
    except Exception as e:  
        print(f"[key_handlers] Error running command {cmd}: {e}", flush=True)  
  
  
# =============================================================================  
# PT_BR: FUNÇÃO DE SIMULAÇÃO DE TECLA  
#         Abstrai a diferença entre xdotool (X11) e ydotool (Wayland).  
#  
# EN_US: KEY SIMULATION FUNCTION  
#         Abstracts the difference between xdotool (X11) and ydotool (Wayland).  
# =============================================================================  
  
def _simulate_key(action, keycode):  
    """  
    PT_BR:  
    Simula o pressionamento de uma tecla usando a ferramenta disponível.  
  
    Estratégia:  
      - X11:                xdotool key <nome_da_tecla>  
      - Wayland + ydotool 0.x:  ydotool key <keycode>  
      - Wayland + ydotool 1.x+: ydotool key <keycode>:1 <keycode>:0  
  
    Args:  
        action:  nome da tecla para xdotool (ex: "Insert", "Home", "Prior")  
        keycode: código numérico do kernel Linux para ydotool (ex: 110, 102, 104)  
  
    EN_US:  
    Simulates a key press using the available tool.  
  
    Strategy:  
      - X11:                xdotool key <key_name>  
      - Wayland + ydotool 0.x:  ydotool key <keycode>  
      - Wayland + ydotool 1.x+: ydotool key <keycode>:1 <keycode>:0  
  
    Args:  
        action:  key name for xdotool (e.g., "Insert", "Home", "Prior")  
        keycode: Linux kernel numeric keycode for ydotool (e.g., 110, 102, 104)  
    """  
    if SIM_TOOL == "xdotool":  
        _run_cmd(["xdotool", "key", action])  
    elif SIM_TOOL == "ydotool":  
        if YDOTOOL_LEGACY:  
            # PT_BR: Versão 0.x -- press/release implícitos, só passa o keycode.  
            # EN_US: Version 0.x -- press/release implicit, just pass the keycode.  
            _run_cmd(["ydotool", "key", str(keycode)])  
        else:  
            # PT_BR: Versão 1.x+ -- press e release explícitos.  
            #         <keycode>:1 = press, <keycode>:0 = release  
            # EN_US: Version 1.x+ -- explicit press and release.  
            #         <keycode>:1 = press, <keycode>:0 = release  
            _run_cmd(["ydotool", "key", f"{keycode}:1", f"{keycode}:0"])  
  
# =============================================================================  
# PT_BR: DEFINIÇÃO DOS SCHEMAS DE PÁGINAS  
#  
#   Cada página é um dicionário que mapeia ButtonKey -> config dict.  
#   Cada config dict contém:  
#     - "action":  nome da tecla para xdotool (str)  
#     - "keycode": código numérico do kernel Linux para ydotool (int)  
#     - "label":   nome amigável para exibição no terminal (str)  
#     - "image":   caminho absoluto para o ícone 64x64 da tecla (str ou None)  
#  
#   Layout físico do K1 PRO (3 colunas x 2 linhas):  
#     +--------+--------+--------+  
#     | KEY_1  | KEY_2  | KEY_3  |  
#     | (1x1)  | (1x2)  | (1x3)  |  
#     +--------+--------+--------+  
#     | KEY_4  | KEY_5  | KEY_6  |  
#     | (2x1)  | (2x2)  | (2x3)  |  
#     +--------+--------+--------+  
#  
# EN_US: PAGE SCHEMA DEFINITIONS  
#  
#   Each page is a dictionary mapping ButtonKey -> config dict.  
#   Each config dict contains:  
#     - "action":  key name for xdotool (str)  
#     - "keycode": Linux kernel numeric keycode for ydotool (int)  
#     - "label":   friendly name for terminal display (str)  
#     - "image":   absolute path to the 64x64 key icon (str or None)  
#  
#   K1 PRO physical layout (3 columns x 2 rows):  
#     +--------+--------+--------+  
#     | KEY_1  | KEY_2  | KEY_3  |  
#     | (1x1)  | (1x2)  | (1x3)  |  
#     +--------+--------+--------+  
#     | KEY_4  | KEY_5  | KEY_6  |  
#     | (2x1)  | (2x2)  | (2x3)  |  
#     +--------+--------+--------+  
#  
# =============================================================================  
  
# PT_BR: Keycodes do kernel Linux (de linux/input-event-codes.h)  
# EN_US: Linux kernel keycodes (from linux/input-event-codes.h)  
#  
# KEY_INSERT   = 110  
# KEY_HOME     = 102  
# KEY_PAGEUP   = 104  
# KEY_DELETE    = 111  
# KEY_END      = 107  
# KEY_PAGEDOWN = 109  
  
_DEFAULT_IMAGES_DIR = os.path.join(_IMAGES_DIR, "default_keyboard_schema")  
  
default_keyboard_schema = {  
    ButtonKey.KEY_1: {  
        "action": "Insert",  
        "keycode": 110,  
        "label": "INSERT",  
        "image": os.path.join(_DEFAULT_IMAGES_DIR, "icons8-insert-button-64.png"),  
    },  
    ButtonKey.KEY_2: {  
        "action": "Home",  
        "keycode": 102,  
        "label": "HOME",  
        "image": os.path.join(_DEFAULT_IMAGES_DIR, "icons8-home-button-64.png"),  
    },  
    ButtonKey.KEY_3: {  
        "action": "Prior",  
        "keycode": 104,  
        "label": "PAGE UP",  
        "image": os.path.join(_DEFAULT_IMAGES_DIR, "icons8-page-up-button-64.png"),  
    },  
    ButtonKey.KEY_4: {  
        "action": "Delete",  
        "keycode": 111,  
        "label": "DELETE",  
        "image": os.path.join(_DEFAULT_IMAGES_DIR, "icons8-delete-key-64.png"),  
    },  
    ButtonKey.KEY_5: {  
        "action": "End",  
        "keycode": 107,  
        "label": "END",  
        "image": os.path.join(_DEFAULT_IMAGES_DIR, "icons8-end-button-64.png"),  
    },  
    ButtonKey.KEY_6: {  
        "action": "Next",  
        "keycode": 109,  
        "label": "PAGE DOWN",  
        "image": os.path.join(_DEFAULT_IMAGES_DIR, "icons8-page-down-button-64.png"),  
    },  
}  
  
  
# =============================================================================  
# PT_BR: REGISTRO DE PÁGINAS  
# EN_US: PAGE REGISTRY  
# =============================================================================  
  
PAGES = {  
    "default_keyboard_schema": default_keyboard_schema,  
}  
  
current_page = "default_keyboard_schema"  
  
  
# =============================================================================  
# PT_BR: HANDLER PRINCIPAL DAS TECLAS  
# EN_US: MAIN KEY HANDLER  
# =============================================================================  
  
def handle_key(key, state):  
    """  
    PT_BR:  
    Handler principal das teclas. Chamado pelo callback do k1pro_linux.py.  
  
    Comportamento:  
      - Só executa no press (state == 1), ignora release (state == 0).  
      - Busca a config da tecla na página ativa.  
      - Usa _simulate_key() que escolhe xdotool ou ydotool automaticamente.  
  
    Args:  
        key:   ButtonKey enum (KEY_1 a KEY_6)  
        state: 1 = pressionado, 0 = solto  
  
    EN_US:  
    Main key handler. Called by the k1pro_linux.py callback.  
  
    Behavior:  
      - Only executes on press (state == 1), ignores release (state == 0).  
      - Looks up the key config in the active page.  
      - Uses _simulate_key() which picks xdotool or ydotool automatically.  
  
    Args:  
        key:   ButtonKey enum (KEY_1 to KEY_6)  
        state: 1 = pressed, 0 = released  
    """  
    if state != 1:  
        return  
  
    page_map = PAGES.get(current_page)  
    if not page_map:  
        print(f"[key_handlers] Page '{current_page}' not found.", flush=True)  
        return  
  
    config = page_map.get(key)  
    if not config:  
        key_num = key.value if hasattr(key, "value") else key  
        print(f"[key_handlers] Key {key_num} not mapped on page '{current_page}'.", flush=True)  
        return  
  
    action = config.get("action")  
    keycode = config.get("keycode")  
    label = config.get("label", action)  
    key_num = key.value if hasattr(key, "value") else key  
  
    if not action and not keycode:  
        print(f"[key_handlers] Key {key_num} ({label}): no action/keycode defined.", flush=True)  
        return  
  
    print(f"Key {key_num} ({label}): {action} [via {SIM_TOOL or 'none'}]", flush=True)  
    _simulate_key(action, keycode)  
  
  
# =============================================================================  
# PT_BR: APLICAR IMAGENS NAS TECLAS LCD  
# EN_US: APPLY IMAGES TO LCD KEYS  
# =============================================================================  
  
def apply_page_images(device, page_name=None):  
    """  
    PT_BR:  
    Aplica as imagens da página especificada nas teclas LCD do K1 PRO.  
    Se page_name for None, usa a página ativa (current_page).  
    Chama device.set_key_image() para cada tecla e device.refresh() ao final.  
    As imagens são gravadas na RAM do dispositivo (perdidas ao desligar).  
  
    Args:  
        device:    instância K1Pro (já aberta e inicializada)  
        page_name: nome da página. Se None, usa current_page.  
  
    EN_US:  
    Applies the specified page's images to the K1 PRO's LCD keys.  
    If page_name is None, uses the active page (current_page).  
    Calls device.set_key_image() for each key and device.refresh() at the end.  
    Images are written to the device's RAM (lost on power off).  
  
    Args:  
        device:    K1Pro instance (already opened and initialized)  
        page_name: page name. If None, uses current_page.  
    """  
    target_page = page_name or current_page  
    page_map = PAGES.get(target_page)  
  
    if not page_map:  
        print(f"[key_handlers] Page '{target_page}' not found. Cannot apply images.", flush=True)  
        return  
  
    print(f"[key_handlers] Applying images for page '{target_page}'...", flush=True)  
  
    for key, config in page_map.items():  
        image_path = config.get("image")  
        if not image_path:  
            continue  
  
        if not os.path.exists(image_path):  
            print(f"  WARNING: Image not found: {image_path}", flush=True)  
            continue  
  
        label = config.get("label", "?")  
        key_num = key.value if hasattr(key, "value") else key  
        print(f"  Key {key_num} ({label}): {os.path.basename(image_path)}", flush=True)  
        device.set_key_image(key_num, image_path)  
  
    device.refresh()  
    print(f"[key_handlers] Images applied for page '{target_page}'.", flush=True)  
  
  
# =============================================================================  
# PT_BR: FUNÇÕES DE NAVEGAÇÃO DE PÁGINAS  
# EN_US: PAGE NAVIGATION FUNCTIONS  
# =============================================================================  
  
def switch_page(page_name):  
    """  
    PT_BR: Troca a página ativa. Retorna True se a página existe, False caso contrário.  
    EN_US: Switches the active page. Returns True if the page exists, False otherwise.  
    """  
    global current_page  
    if page_name not in PAGES:  
        print(f"[key_handlers] Page '{page_name}' not found. Available: {list(PAGES.keys())}", flush=True)  
        return False  
    current_page = page_name  
    print(f"[key_handlers] Switched to page '{current_page}'", flush=True)  
    return True  
  
  
def get_page_names():  
    """  
    PT_BR: Retorna a lista de nomes de páginas disponíveis.  
    EN_US: Returns the list of available page names.  
    """  
    return list(PAGES.keys())  
  
  
def get_current_page_name():  
    """  
    PT_BR: Retorna o nome da página ativa.  
    EN_US: Returns the name of the active page.  
    """  
    return current_page