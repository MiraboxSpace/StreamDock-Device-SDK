#pragma once
#include <map>
#include "StreamDock293.h"
#include "StreamDock293V3.h"
#include "StreamDockN3V25.h"
#include "StreamDockN4.h"
#include "StreamDockN1.h"
enum USBVendorIDs {
    USB_VID = 0x5500,
    USB_VID293s = 0x5548,
    USB_VID293V3 = 0x6603,
    USB_VIDN3_1 = 0x6602,
    USB_VIDN3 = 0x6603,
    USB_VIDN3V2 = 0xEEEF,
    USB_VIDN4 = 0x6602,
    USB_VIDN4EN = 0x6603,
    USB_VIDN1 = 0x6603

};

enum USBProductIDs {
    USB_PID_STREAMDOCK_936 = 0x1001,
    USB_PID_STREAMDOCK_936s = 0x6670,
    USB_PID_STREAMDOCK_293V3 = 0x1005,
    USB_PID_STREAMDOCK_293V3EN = 0x1006,
    USB_PID_STREAMDOCK_293V25 = 0x1010,
    USB_PID_STREAMDOCK_N3_1 = 0x1000,
    USB_PID_STREAMDOCK_N3 = 0x1002,
    USB_PID_STREAMDOCK_N3EN = 0x1003,
    USB_PID_STREAMDOCK_N3V2 = 0x2929,
    USB_PID_STREAMDOCK_N4 = 0x1001,
    USB_PID_STREAMDOCK_N4EN = 0x1007,
    USB_PID_STREAMDOCK_N1 = 0x1000
};
enum DeivceType
{
    Device293,
    Device293s,
    Device293V3,
    Device293N3V25,
    Device293N4,
    Device293N1,
};

const std::map<std::pair<USBVendorIDs, USBProductIDs>, DeivceType> StreamDockDevices = {
    {{USB_VID, USB_PID_STREAMDOCK_936}, Device293},
    {{USB_VID293s, USB_PID_STREAMDOCK_936s}, Device293s},
    {{USB_VID293V3, USB_PID_STREAMDOCK_293V3}, Device293V3},
    {{USB_VID293V3, USB_PID_STREAMDOCK_293V3EN}, Device293V3},
    {{USB_VID293V3, USB_PID_STREAMDOCK_293V25}, Device293V3},
    {{USB_VIDN3, USB_PID_STREAMDOCK_N3}, Device293N3V25},
    {{USB_VIDN4, USB_PID_STREAMDOCK_N4}, Device293N4},
    {{USB_VIDN4, USB_PID_STREAMDOCK_N4EN}, Device293N4},
    {{USB_VIDN1, USB_PID_STREAMDOCK_N1}, Device293N1},
};
