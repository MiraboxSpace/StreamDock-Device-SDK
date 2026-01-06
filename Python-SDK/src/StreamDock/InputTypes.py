"""
StreamDock input event type system

Provides unified input event definitions, including buttons, knobs, and swipe gestures.
"""

from enum import Enum, IntEnum
from dataclasses import dataclass
from typing import Optional


class EventType(Enum):
    """Event type enum"""
    BUTTON = "button"           # Button press/release
    KNOB_ROTATE = "knob_rotate" # Knob rotation
    KNOB_PRESS = "knob_press"   # Knob press
    SWIPE = "swipe"             # Swipe gesture
    UNKNOWN = "unknown"


class ButtonKey(IntEnum):
    """
    Logical key values for regular buttons (used for setting images)

    Devices can define their own key ranges, for example:
    - Simple devices: KEY_1 ~ KEY_15
    - XL devices: KEY_1 ~ KEY_32
    """
    KEY_1 = 1
    KEY_2 = 2
    KEY_3 = 3
    KEY_4 = 4
    KEY_5 = 5
    KEY_6 = 6
    KEY_7 = 7
    KEY_8 = 8
    KEY_9 = 9
    KEY_10 = 10
    KEY_11 = 11
    KEY_12 = 12
    KEY_13 = 13
    KEY_14 = 14
    KEY_15 = 15
    KEY_16 = 16
    KEY_17 = 17
    KEY_18 = 18
    KEY_19 = 19
    KEY_20 = 20
    KEY_21 = 21
    KEY_22 = 22
    KEY_23 = 23
    KEY_24 = 24
    KEY_25 = 25
    KEY_26 = 26
    KEY_27 = 27
    KEY_28 = 28
    KEY_29 = 29
    KEY_30 = 30
    KEY_31 = 31
    KEY_32 = 32


class KnobId(Enum):
    """Knob ID enum"""
    KNOB_1 = "knob_1"
    KNOB_2 = "knob_2"
    KNOB_3 = "knob_3"
    KNOB_4 = "knob_4"


class Direction(Enum):
    """Direction enum (for knob rotation and swipe gestures)"""
    LEFT = "left"
    RIGHT = "right"


@dataclass
class InputEvent:
    """
    Unified input event class

    All input events (buttons, knobs, swipes) are passed to callbacks through this class.

    Attributes:
        event_type: Event type
        key: Button event: which key
        knob_id: Knob event: which knob
        direction: Direction: knob rotation direction or swipe direction
        state: State: 0=release, 1=press
    """
    event_type: EventType
    key: Optional[ButtonKey] = None      # Button event: which key
    knob_id: Optional[KnobId] = None     # Knob event: which knob
    direction: Optional[Direction] = None # Direction: knob rotation direction or swipe direction
    state: int = 0                       # State: 0=release, 1=press

    def __post_init__(self):
        """Data validation"""
        if self.event_type == EventType.BUTTON:
            if self.key is None:
                raise ValueError("BUTTON event requires key")
        elif self.event_type in (EventType.KNOB_ROTATE, EventType.KNOB_PRESS):
            if self.knob_id is None:
                raise ValueError("KNOB event requires knob_id")
            if self.event_type == EventType.KNOB_ROTATE and self.direction is None:
                raise ValueError("KNOB_ROTATE event requires direction")
        elif self.event_type == EventType.SWIPE:
            if self.direction is None:
                raise ValueError("SWIPE event requires direction")
