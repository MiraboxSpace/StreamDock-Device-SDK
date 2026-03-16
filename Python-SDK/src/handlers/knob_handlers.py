# knob_handlers.py  
#  
######################################################################################  
#  
# PT_BR:  
# Handlers globais dos 3 knobs do K1 PRO.  
#  
# DEBOUNCE COM ACUMULAÇÃO:  
#   Quando o knob é girado rápido, vários eventos chegam em sequência.  
#   Em vez de executar um comando pactl para cada tick individual,  
#   acumulamos os ticks (+1 para direita, -1 para esquerda) e esperamos  
#   o knob "estabilizar" (parar de girar por DEBOUNCE_SECONDS).  
#   Quando estabiliza, calculamos o saldo líquido e aplicamos UMA ÚNICA  
#   mudança de volume proporcional ao deslocamento total.  
#  
#   Exemplo: girou rápido 8 ticks para direita e 2 para esquerda  
#            saldo = +6 → aplica +30% de uma vez (6 * 5%)  
#  
# EN_US:  
# Global handlers for the K1 PRO's 3 knobs.  
#  
# DEBOUNCE WITH ACCUMULATION:  
#   When the knob is rotated quickly, multiple events arrive in sequence.  
#   Instead of executing a pactl command for each individual tick,  
#   we accumulate ticks (+1 for right, -1 for left) and wait for  
#   the knob to "stabilize" (stop rotating for DEBOUNCE_SECONDS).  
#   When it stabilizes, we calculate the net displacement and apply  
#   a SINGLE volume change proportional to the total displacement.  
#  
#   Example: quickly rotated 8 ticks right and 2 left  
#            net = +6 → applies +30% at once (6 * 5%)  
#  
######################################################################################  
  
import subprocess  
import threading  
from StreamDock.InputTypes import EventType, Direction  
  
  
######################################################################################  
#  
# PT_BR: CONFIGURAÇÕES  
#  
# VOLUME_STEP_INT: valor inteiro do step de volume em pontos percentuais.  
#   Cada tick líquido do knob muda o volume em VOLUME_STEP_INT%.  
#   O total aplicado é: saldo_liquido * VOLUME_STEP_INT  
#  
# DEBOUNCE_SECONDS: tempo em segundos para esperar após o último tick  
#   antes de aplicar a mudança. 0.05 = 50ms é um bom valor.  
#   - Muito baixo (ex: 0.01): não acumula bem, quase igual a sem debounce.  
#   - Muito alto (ex: 0.3): demora para responder, sensação de lag.  
#   - 0.05 (50ms): bom equilíbrio entre responsividade e acumulação.  
#  
# EN_US: SETTINGS  
#  
# VOLUME_STEP_INT: integer value of the volume step in percentage points.  
#   Each net knob tick changes volume by VOLUME_STEP_INT%.  
#   Total applied is: net_displacement * VOLUME_STEP_INT  
#  
# DEBOUNCE_SECONDS: time in seconds to wait after the last tick  
#   before applying the change. 0.05 = 50ms is a good value.  
#   - Too low (e.g., 0.01): doesn't accumulate well, almost like no debounce.  
#   - Too high (e.g., 0.3): slow to respond, feels laggy.  
#   - 0.05 (50ms): good balance between responsiveness and accumulation.  
#  
######################################################################################  
  
VOLUME_STEP_INT = 12.5  
DEBOUNCE_SECONDS = 0.05  
  
  
######################################################################################  
#  
# PT_BR: ESTADO DO DEBOUNCE  
#  
# _accumulators: dicionário {chave: int} que guarda o saldo de ticks por knob.  
#   Chave é uma string identificadora (ex: "knob_2_rotate", "knob_3_rotate").  
#   Valor é o saldo: positivo = mais ticks para direita, negativo = mais para esquerda.  
#  
# _timers: dicionário {chave: Timer} com o timer de debounce ativo por knob.  
#   Cada novo tick cancela o timer anterior e cria um novo.  
#   Quando o timer expira (knob parou), a função de aplicação é chamada.  
#  
# _lock: mutex para proteger _accumulators e _timers de acesso concorrente.  
#   Necessário porque os callbacks do SDK rodam em uma thread e os timers  
#   expiram em outra.  
#  
# EN_US: DEBOUNCE STATE  
#  
# _accumulators: dict {key: int} storing the tick balance per knob.  
#   Key is an identifier string (e.g., "knob_2_rotate", "knob_3_rotate").  
#   Value is the balance: positive = more right ticks, negative = more left ticks.  
#  
# _timers: dict {key: Timer} with the active debounce timer per knob.  
#   Each new tick cancels the previous timer and creates a new one.  
#   When the timer expires (knob stopped), the apply function is called.  
#  
# _lock: mutex to protect _accumulators and _timers from concurrent access.  
#   Needed because SDK callbacks run in one thread and timers expire in another.  
#  
######################################################################################  
  
_accumulators = {}  
_timers = {}  
_lock = threading.Lock()  
  
  
# =============================================================================  
# PT_BR: FUNÇÕES AUXILIARES  
# EN_US: HELPER FUNCTIONS  
# =============================================================================  
  
def _run_cmd(cmd):  
    """  
    PT_BR: Executa comando de forma assíncrona (não-bloqueante).  
    EN_US: Executes command asynchronously (non-blocking).  
    """  
    try:  
        subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)  
    except Exception as e:  
        print(f"Command error: {e}", flush=True)  
  
  
def _accumulate(key, delta, apply_fn):  
    """  
    PT_BR:  
    Acumula um tick de knob e agenda a aplicação após debounce.  
  
    Fluxo:  
      1. Soma delta (+1 ou -1) ao acumulador do knob identificado por key.  
      2. Cancela o timer anterior (se existir) -- o knob ainda está girando.  
      3. Cria um novo timer de DEBOUNCE_SECONDS.  
      4. Quando o timer expira (knob parou de girar):  
         a. Lê e zera o acumulador.  
         b. Se o saldo líquido != 0, chama apply_fn(saldo).  
  
    Args:  
        key:      string identificadora do knob (ex: "knob_3_rotate")  
        delta:    +1 para direita, -1 para esquerda  
        apply_fn: função que recebe o saldo líquido (int) e aplica a ação.  
                  Exemplo: _apply_speaker_volume(+6) → pactl +30%  
  
    EN_US:  
    Accumulates a knob tick and schedules application after debounce.  
  
    Flow:  
      1. Adds delta (+1 or -1) to the accumulator for the knob identified by key.  
      2. Cancels the previous timer (if any) -- the knob is still rotating.  
      3. Creates a new timer of DEBOUNCE_SECONDS.  
      4. When the timer expires (knob stopped rotating):  
         a. Reads and resets the accumulator.  
         b. If net balance != 0, calls apply_fn(balance).  
  
    Args:  
        key:      identifier string for the knob (e.g., "knob_3_rotate")  
        delta:    +1 for right, -1 for left  
        apply_fn: function that receives the net balance (int) and applies the action.  
                  Example: _apply_speaker_volume(+6) → pactl +30%  
    """  
    with _lock:  
        # PT_BR: Passo 1 - Acumula o tick  
        # EN_US: Step 1 - Accumulate the tick  
        _accumulators[key] = _accumulators.get(key, 0) + delta  
  
        # PT_BR: Passo 2 - Cancela timer anterior (knob ainda girando)  
        # EN_US: Step 2 - Cancel previous timer (knob still rotating)  
        old_timer = _timers.get(key)  
        if old_timer is not None:  
            old_timer.cancel()  
  
        # PT_BR: Passo 3 - Cria novo timer  
        # EN_US: Step 3 - Create new timer  
        def _fire():  
            with _lock:  
                net = _accumulators.pop(key, 0)  
                _timers.pop(key, None)  
            if net != 0:  
                apply_fn(net)  
  
        timer = threading.Timer(DEBOUNCE_SECONDS, _fire)  
        timer.daemon = True  
        timer.start()  
        _timers[key] = timer  
  
  
# =============================================================================  
# PT_BR: FUNÇÕES DE APLICAÇÃO DE VOLUME  
#         Recebem o saldo líquido de ticks e executam UMA chamada pactl.  
#  
# EN_US: VOLUME APPLICATION FUNCTIONS  
#         Receive the net tick balance and execute ONE pactl call.  
# =============================================================================  
  
def _apply_speaker_volume(net_ticks):  
    """  
    PT_BR:  
    Aplica mudança de volume de saída proporcional ao saldo de ticks.  
    net_ticks > 0 → aumenta volume.  
    net_ticks < 0 → diminui volume.  
    Total = |net_ticks| * VOLUME_STEP_INT %  
  
    Exemplo: net_ticks = +6, VOLUME_STEP_INT = 5 → pactl +30%  
    Exemplo: net_ticks = -3, VOLUME_STEP_INT = 5 → pactl -15%  
  
    EN_US:  
    Applies output volume change proportional to the tick balance.  
    net_ticks > 0 → increase volume.  
    net_ticks < 0 → decrease volume.  
    Total = |net_ticks| * VOLUME_STEP_INT %  
    """  
    total = abs(net_ticks) * VOLUME_STEP_INT  
    sign = "+" if net_ticks > 0 else "-"  
    _run_cmd(["pactl", "set-sink-volume", "@DEFAULT_SINK@", f"{sign}{total}%"])  
  
  
def _apply_mic_volume(net_ticks):  
    """  
    PT_BR:  
    Aplica mudança de volume do microfone proporcional ao saldo de ticks.  
    Mesma lógica de _apply_speaker_volume, mas para @DEFAULT_SOURCE@.  
  
    EN_US:  
    Applies microphone volume change proportional to the tick balance.  
    Same logic as _apply_speaker_volume, but for @DEFAULT_SOURCE@.  
    """  
    total = abs(net_ticks) * VOLUME_STEP_INT  
    sign = "+" if net_ticks > 0 else "-"  
    _run_cmd(["pactl", "set-source-volume", "@DEFAULT_SOURCE@", f"{sign}{total}%"])  
  
  
# =============================================================================  
# KNOB 1 - PLACEHOLDER  
# =============================================================================  
def handle_knob_1(event_type, direction=None, state=None):  
    """  
    PT_BR: Handler do Knob 1. Sem ação definida, apenas debug.  
    EN_US: Handler for Knob 1. No action defined, debug only.  
    """  
    if event_type == EventType.KNOB_ROTATE:  
        if direction == Direction.RIGHT:  
            print("Knob 1: rotate right (no action defined)", flush=True)  
        elif direction == Direction.LEFT:  
            print("Knob 1: rotate left (no action defined)", flush=True)  
    elif event_type == EventType.KNOB_PRESS:  
        if state == 1:  
            print("Knob 1: pressed (no action defined)", flush=True)  
  
  
# =============================================================================  
# KNOB 2 - VOLUME DO MICROFONE (COM DEBOUNCE)  
# =============================================================================  
def handle_knob_2(event_type, direction=None, state=None):  
    """  
    PT_BR:  
    Handler do Knob 2 - controle de volume do microfone com debounce.  
  
    Rotação: acumula ticks e aplica o saldo líquido após estabilização.  
    Pressionar: mute/unmute imediato (sem debounce, é um evento único).  
  
    EN_US:  
    Handler for Knob 2 - microphone volume control with debounce.  
  
    Rotation: accumulates ticks and applies net balance after stabilization.  
    Press: immediate mute/unmute (no debounce, it's a single event).  
    """  
    if event_type == EventType.KNOB_ROTATE:  
        # PT_BR: Converte direção em delta: RIGHT = +1, LEFT = -1  
        # EN_US: Converts direction to delta: RIGHT = +1, LEFT = -1  
        delta = +1 if direction == Direction.RIGHT else -1  
        _accumulate("knob_2_rotate", delta, _apply_mic_volume)  
  
    elif event_type == EventType.KNOB_PRESS:  
        if state == 1:  
            # PT_BR: Mute/unmute é imediato, não precisa de debounce.  
            # EN_US: Mute/unmute is immediate, no debounce needed.  
            _run_cmd(["pactl", "set-source-mute", "@DEFAULT_SOURCE@", "toggle"])  
  
  
# =============================================================================  
# KNOB 3 - VOLUME DE SAÍDA (COM DEBOUNCE)  
# =============================================================================  
def handle_knob_3(event_type, direction=None, state=None):  
    """  
    PT_BR:  
    Handler do Knob 3 - controle de volume de saída com debounce.  
  
    Rotação: acumula ticks e aplica o saldo líquido após estabilização.  
    Pressionar: mute/unmute imediato (sem debounce).  
  
    EN_US:  
    Handler for Knob 3 - output volume control with debounce.  
  
    Rotation: accumulates ticks and applies net balance after stabilization.  
    Press: immediate mute/unmute (no debounce).  
    """  
    if event_type == EventType.KNOB_ROTATE:  
        # PT_BR: Converte direção em delta: RIGHT = +1, LEFT = -1  
        # EN_US: Converts direction to delta: RIGHT = +1, LEFT = -1  
        delta = +1 if direction == Direction.RIGHT else -1  
        _accumulate("knob_3_rotate", delta, _apply_speaker_volume)  
  
    elif event_type == EventType.KNOB_PRESS:  
        if state == 1:  
            # PT_BR: Mute/unmute é imediato, não precisa de debounce.  
            # EN_US: Mute/unmute is immediate, no debounce needed.  
            _run_cmd(["pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle"])