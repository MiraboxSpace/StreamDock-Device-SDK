"""
StreamDock 输入事件类型系统

提供统一的输入事件定义，包括按键、旋钮、滑动手势等。
"""

from enum import Enum, IntEnum
from dataclasses import dataclass
from typing import Optional


class EventType(Enum):
    """事件类型枚举"""
    BUTTON = "button"           # 普通按键按下/抬起
    KNOB_ROTATE = "knob_rotate" # 旋钮旋转
    KNOB_PRESS = "knob_press"   # 旋钮按下
    SWIPE = "swipe"             # 滑动手势
    UNKNOWN = "unknown"


class ButtonKey(IntEnum):
    """
    普通按键的逻辑键值（用于设置图片）

    设备可以定义自己的按键范围，例如：
    - 简单设备: KEY_1 ~ KEY_15
    - XL 设备: KEY_1 ~ KEY_32
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
    """旋钮ID枚举"""
    KNOB_1 = "knob_1"
    KNOB_2 = "knob_2"
    KNOB_3 = "knob_3"
    KNOB_4 = "knob_4"


class Direction(Enum):
    """方向枚举（用于旋钮旋转和滑动手势）"""
    LEFT = "left"
    RIGHT = "right"


@dataclass
class InputEvent:
    """
    统一输入事件类

    所有输入事件（按键、旋钮、滑动）都通过此类传递给回调函数。

    Attributes:
        event_type: 事件类型
        key: 按键事件：哪个按键
        knob_id: 旋钮事件：哪个旋钮
        direction: 方向：旋钮旋转方向或滑动方向
        state: 状态：0=抬起/释放, 1=按下
    """
    event_type: EventType
    key: Optional[ButtonKey] = None      # 按键事件：哪个按键
    knob_id: Optional[KnobId] = None     # 旋钮事件：哪个旋钮
    direction: Optional[Direction] = None # 方向：旋钮旋转方向或滑动方向
    state: int = 0                       # 状态：0=抬起/释放, 1=按下

    def __post_init__(self):
        """数据校验"""
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
