class USBVendorIDs:
    """
    USB Vendor IDs for known StreamDock devices.
    """
    USB_VID_293 = 0x5500
    USB_VID_293s = 0x5548
    USB_PID_293V3 = 0x6603
    USB_VIDN3 = 0x6603
    USB_VIDN3V2 = 0xEEEF
    USB_VIDN3V25 = 0x1500
    USB_VIDN3E = 0x6602
    USB_VIDN4 = 0x6602
    USB_VIDN4EN = 0x6603
    USB_VIDN1EN = 0x6603
    USB_VIDN1 = 0x6603


class USBProductIDs:
    """
    USB Product IDs for known StreamDock devices.
    """
    USB_PID_STREAMDOCK_293 = 0x1001
    USB_PID_STREAMDOCK_293s = 0x6670
    USB_PID_STREAMDOCK_293V3 = 0x1005
    USB_PID_STREAMDOCK_293V3EN = 0x1006
    USB_PID_STREAMDOCK_293V25 = 0x1010
    USB_PID_STREAMDOCK_N3 = 0x1002
    USB_PID_STREAMDOCK_N3EN = 0x1003
    USB_PID_STREAMDOCK_N3V2 = 0x2929
    USB_PID_STREAMDOCK_N3V25 = 0x3001
    USB_PID_STREAMDOCK_N4 = 0x1001
    USB_PID_STREAMDOCK_N4EN = 0x1007
    USB_PID_STREAMDOCK_N1EN = 0x1000
    USB_PID_STREAMDOCK_N1 = 0x1011

from .Devices.StreamDock293 import StreamDock293
from .Devices.StreamDock293s import StreamDock293s
from .Devices.StreamDock293V3 import StreamDock293V3
from .Devices.StreamDockN3 import StreamDockN3
from .Devices.StreamDockN4 import StreamDockN4
from .Devices.StreamDockN1 import StreamDockN1
g_products = [
    # 293 serial
    (USBVendorIDs.USB_VID_293, USBProductIDs.USB_PID_STREAMDOCK_293, StreamDock293),
    (USBVendorIDs.USB_VID_293s, USBProductIDs.USB_PID_STREAMDOCK_293s, StreamDock293s),
    (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V3, StreamDock293V3),
    (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V3EN, StreamDock293V3),
    (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V25, StreamDock293V3),
    # N3
    (USBVendorIDs.USB_VIDN3, USBProductIDs.USB_PID_STREAMDOCK_N3, StreamDockN3),
    (USBVendorIDs.USB_VIDN3, USBProductIDs.USB_PID_STREAMDOCK_N3EN, StreamDockN3),
    (USBVendorIDs.USB_VIDN3E, USBProductIDs.USB_PID_STREAMDOCK_N3, StreamDockN3),
    (USBVendorIDs.USB_VIDN3E, USBProductIDs.USB_PID_STREAMDOCK_N3EN, StreamDockN3),
    (USBVendorIDs.USB_VIDN3E, USBProductIDs.USB_PID_STREAMDOCK_N3V2, StreamDockN3),
    (USBVendorIDs.USB_VIDN3V25, USBProductIDs.USB_PID_STREAMDOCK_N3V25, StreamDockN3),
    # N4
    (USBVendorIDs.USB_VIDN4, USBProductIDs.USB_PID_STREAMDOCK_N4, StreamDockN4),
    (USBVendorIDs.USB_VIDN4EN, USBProductIDs.USB_PID_STREAMDOCK_N4EN, StreamDockN4),

    #N1
    (USBVendorIDs.USB_VIDN1, USBProductIDs.USB_PID_STREAMDOCK_N1, StreamDockN1),
    (USBVendorIDs.USB_VIDN1EN, USBProductIDs.USB_PID_STREAMDOCK_N1EN, StreamDockN1),
]