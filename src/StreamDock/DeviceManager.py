from .Devices.StreamDock293 import StreamDock293
from .Devices.StreamDock293s import StreamDock293s
from .Devices.StreamDock293V3 import StreamDock293V3
from .ProductIDs import USBVendorIDs, USBProductIDs
from .Transport.LibUSBHIDAPI import LibUSBHIDAPI
# import pyudev

class DeviceManager:
    streamdocks = list()
    @staticmethod
    def _get_transport(transport):

        return LibUSBHIDAPI()

    def __init__(self, transport=None):

        self.transport = self._get_transport(transport)

    def enumerate(self):
        products = [
            (USBVendorIDs.USB_VID_293, USBProductIDs.USB_PID_STREAMDOCK_293, StreamDock293),
            (USBVendorIDs.USB_VID_293s, USBProductIDs.USB_PID_STREAMDOCK_293s, StreamDock293s),
            (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V3, StreamDock293V3),
            (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V3EN, StreamDock293V3),
            (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V25, StreamDock293V3),
            # (USBVendorIDs.USB_VIDN3, USBProductIDs.USB_PID_STREAMDOCK_N3, StreamDockN3),
            # (USBVendorIDs.USB_VIDN3, USBProductIDs.USB_PID_STREAMDOCK_N3EN, StreamDockN3),
            # (USBVendorIDs.USB_VIDN4, USBProductIDs.USB_PID_STREAMDOCK_N4, StreamDockN4),
            # (USBVendorIDs.USB_VIDN4EN, USBProductIDs.USB_PID_STREAMDOCK_N4EN, StreamDockN4)
        ]


        for vid, pid, class_type in products:
            found_devices = self.transport.enumerate(vid=vid, pid=pid)
            self.streamdocks.extend([class_type(self.transport,d) for d in found_devices])

        return self.streamdocks
    


    # def listen(self):
    #     products = [
    #         (USBVendorIDs.USB_VID_293, USBProductIDs.USB_PID_STREAMDOCK_293, StreamDock293),
    #         (USBVendorIDs.USB_VID_293s, USBProductIDs.USB_PID_STREAMDOCK_293s, StreamDock293s),
    #         (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V3, StreamDock293V3),
    #         (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V3EN, StreamDock293V3),
    #         (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V25, StreamDock293V3),
    #         # (USBVendorIDs.USB_VIDN3, USBProductIDs.USB_PID_STREAMDOCK_N3, StreamDockN3),
    #         # (USBVendorIDs.USB_VIDN3, USBProductIDs.USB_PID_STREAMDOCK_N3EN, StreamDockN3),
    #         # (USBVendorIDs.USB_VIDN4, USBProductIDs.USB_PID_STREAMDOCK_N4, StreamDockN4),
    #         # (USBVendorIDs.USB_VIDN4EN, USBProductIDs.USB_PID_STREAMDOCK_N4EN, StreamDockN4)
    #     ]

    #     context = pyudev.Context()
    #     monitor = pyudev.Monitor.from_netlink(context)
    #     monitor.filter_by(subsystem='usb')
    #     flag1=0
    #     flag2=0
  
    #     for device in iter(monitor.poll, None):
    #         if device.action == 'add' or device.action == 'remove':
    #             if  device.action == 'add':
                    
    #                 if flag1==0:
    #                     flag1=1
    #                     vendor_id = device.get('ID_VENDOR_ID')
    #                     product_id = device.get('ID_MODEL_ID')
    #                 elif flag1==1:
    #                     flag1=0
    #                     for vid, pid, class_type in products:
    #                         if int(vendor_id, 16)==vid and int(product_id, 16)==pid:
    #                             found_devices = self.transport.enumerate(int(vendor_id, 16), int(product_id, 16))
    #                             # print(device.device_path)
    #                             for current_device in found_devices:
    #                                 if device.device_path.find(current_device['path'])!=-1:
    #                                     self.streamdocks.append(class_type(self.transport,current_device))
    #                                     print("创建成功")
                                    
                                
    #             elif  device.action == 'remove':
    #                 if flag2==0:
    #                     index=0
    #                     for streamdock in self.streamdocks:
    #                         if device.device_path.find(streamdock.getPath())!=-1:
    #                             self.streamdocks.pop(index)
    #                             del streamdock
    #                             print("删除成功")
    #                             break
    #                         index=index+1
    #                     flag2=1
    #                 elif flag2==1:
    #                     flag2=0

####### Windows
# import pywinusb.hid as hid
# from .Devices.StreamDock293 import StreamDock293
# from .Devices.StreamDock293s import StreamDock293s
# from .Devices.StreamDock293V3 import StreamDock293V3
# from .ProductIDs import USBVendorIDs, USBProductIDs
# from .Transport.LibUSBHIDAPI import LibUSBHIDAPI

# class DeviceManager:
#     streamdocks = list()

#     @staticmethod
#     def _get_transport(transport):
#         return LibUSBHIDAPI()

#     def __init__(self, transport=None):
#         self.transport = self._get_transport(transport)

#     def enumerate(self):
#         products = [
#             (USBVendorIDs.USB_VID_293, USBProductIDs.USB_PID_STREAMDOCK_293, StreamDock293),
#             (USBVendorIDs.USB_VID_293s, USBProductIDs.USB_PID_STREAMDOCK_293s, StreamDock293s),
#             (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V3, StreamDock293V3),
#             (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V3EN, StreamDock293V3),
#             (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V25, StreamDock293V3),
#             # (USBVendorIDs.USB_VIDN3, USBProductIDs.USB_PID_STREAMDOCK_N3, StreamDockN3),
#             # (USBVendorIDs.USB_VIDN3, USBProductIDs.USB_PID_STREAMDOCK_N3EN, StreamDockN3),
#             # (USBVendorIDs.USB_VIDN4, USBProductIDs.USB_PID_STREAMDOCK_N4, StreamDockN4),
#             # (USBVendorIDs.USB_VIDN4EN, USBProductIDs.USB_PID_STREAMDOCK_N4EN, StreamDockN4)
#         ]

#         for vid, pid, class_type in products:
#             found_devices = self.transport.enumerate(vid=vid, pid=pid)
#             self.streamdocks.extend([class_type(self.transport, d) for d in found_devices])

#         return self.streamdocks

#     def listen(self):
#         products = [
#             (USBVendorIDs.USB_VID_293, USBProductIDs.USB_PID_STREAMDOCK_293, StreamDock293),
#             (USBVendorIDs.USB_VID_293s, USBProductIDs.USB_PID_STREAMDOCK_293s, StreamDock293s),
#             (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V3, StreamDock293V3),
#             (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V3EN, StreamDock293V3),
#             (USBVendorIDs.USB_PID_293V3, USBProductIDs.USB_PID_STREAMDOCK_293V25, StreamDock293V3),
#             # (USBVendorIDs.USB_VIDN3, USBProductIDs.USB_PID_STREAMDOCK_N3, StreamDockN3),
#             # (USBVendorIDs.USB_VIDN3, USBProductIDs.USB_PID_STREAMDOCK_N3EN, StreamDockN3),
#             # (USBVendorIDs.USB_VIDN4, USBProductIDs.USB_PID_STREAMDOCK_N4, StreamDockN4),
#             # (USBVendorIDs.USB_VIDN4EN, USBProductIDs.USB_PID_STREAMDOCK_N4EN, StreamDockN4)
#         ]

#         # Get all the HID devices
#         all_devices = hid.find_all_hid_devices()

#         for device in all_devices:
#             vendor_id = device.vendor_id
#             product_id = device.product_id
#             for vid, pid, class_type in products:
#                 if vendor_id == vid and product_id == pid:
#                     found_devices = self.transport.enumerate(vid, pid)
#                     for current_device in found_devices:
#                         # Check if device is newly inserted
#                         if device.device_path.find(current_device['path']) != -1:
#                             self.streamdocks.append(class_type(self.transport, current_device))
#                             print("创建成功")

#     def _handle_device_removal(self, device):
#         index = 0
#         for streamdock in self.streamdocks:
#             if device.device_path.find(streamdock.getPath()) != -1:
#                 self.streamdocks.pop(index)
#                 del streamdock
#                 print("删除成功")
#                 break
#             index += 1
