import os
import ctypes
import platform
from ctypes import POINTER, c_void_p, c_char_p, c_int, c_ulong, c_ubyte

# 获取当前平台架构
arch = platform.architecture()[0]

# debug mac arm64
dll_name = 'libtransport_mac_arm64.dylib'

# 获取动态库的绝对路径
dllabspath = os.path.dirname(os.path.abspath(__file__)) + os.path.sep + dll_name

# 加载动态库
my_transport_lib = ctypes.CDLL(dllabspath)


class LibUSBHIDAPI:

    class hid_device_info(ctypes.Structure):
        """
        Structure definition for the hid_device_info structure defined
        in the LibUSB HIDAPI library API.
        """
        pass

    hid_device_info._fields_ = [
        ('path', ctypes.c_char_p),
        ('vendor_id', ctypes.c_ushort),
        ('product_id', ctypes.c_ushort),
        ('serial_number', ctypes.c_wchar_p),
        ('release_number', ctypes.c_ushort),
        ('manufacturer_string', ctypes.c_wchar_p),
        ('product_string', ctypes.c_wchar_p),
        ('usage_page', ctypes.c_ushort),
        ('usage', ctypes.c_ushort),
        ('interface_number', ctypes.c_int),
        ('next', ctypes.POINTER(hid_device_info))
    ]
    my_transport_lib.TranSport_new.restype = c_void_p
    my_transport_lib.TranSport_new.argtypes = []

    my_transport_lib.TranSport_destory.restype = None
    my_transport_lib.TranSport_destory.argtypes = [c_void_p]

    my_transport_lib.TranSport_open_.restype = c_int
    my_transport_lib.TranSport_open_.argtypes = [c_void_p, c_char_p]

    my_transport_lib.TranSport_setBrightness.restype = c_int
    my_transport_lib.TranSport_setBrightness.argtypes = [c_void_p, c_int]

    my_transport_lib.TranSport_read.restype = POINTER(c_ubyte)
    my_transport_lib.TranSport_read.argtypes = [c_void_p]

    my_transport_lib.TranSport_write.restype = c_int
    my_transport_lib.TranSport_write.argtypes = [c_void_p, POINTER(c_ubyte), c_ulong]

    my_transport_lib.TranSport_getInputReport.restype = POINTER(c_ubyte)
    my_transport_lib.TranSport_getInputReport.argtypes = [c_void_p, c_int]

    my_transport_lib.TranSport_freeEnumerate.restype = None
    my_transport_lib.TranSport_freeEnumerate.argtypes = [c_void_p, POINTER(hid_device_info)]

    my_transport_lib.TranSport_enumerate.restype = POINTER(hid_device_info)
    my_transport_lib.TranSport_enumerate.argtypes = [c_void_p, c_int, c_int]

    my_transport_lib.TranSport_setBackgroundImg.restype = c_int
    my_transport_lib.TranSport_setBackgroundImg.argtypes = [c_void_p, POINTER(c_ubyte), c_int]

    my_transport_lib.TranSport_setBackgroundImgDualDevice.restype = c_int
    my_transport_lib.TranSport_setBackgroundImgDualDevice.argtypes = [c_void_p, c_char_p]

    my_transport_lib.TranSport_setKeyImg.restype = c_int
    my_transport_lib.TranSport_setKeyImg.argtypes = [c_void_p, c_char_p, c_int]

    my_transport_lib.TranSport_setKeyImgDualDevice.restype = c_int
    my_transport_lib.TranSport_setKeyImgDualDevice.argtypes = [c_void_p, c_char_p, c_int]

    my_transport_lib.TranSport_setKeyImgDataDualDevice.restype = c_int
    my_transport_lib.TranSport_setKeyImgDataDualDevice.argtypes = [c_void_p, c_char_p, c_int]

    my_transport_lib.TranSport_keyClear.restype = c_int
    my_transport_lib.TranSport_keyClear.argtypes = [c_void_p, c_int]

    my_transport_lib.TranSport_keyAllClear.restype = c_int
    my_transport_lib.TranSport_keyAllClear.argtypes = [c_void_p]

    my_transport_lib.TranSport_wakeScreen.restype = c_int
    my_transport_lib.TranSport_wakeScreen.argtypes = [c_void_p]

    my_transport_lib.TranSport_refresh.restype = c_int
    my_transport_lib.TranSport_refresh.argtypes = [c_void_p]

    my_transport_lib.TranSport_disconnected.restype = c_int
    my_transport_lib.TranSport_disconnected.argtypes = [c_void_p]

    my_transport_lib.TranSport_close.restype = None
    my_transport_lib.TranSport_close.argtypes = [c_void_p]
    def __init__(self):
        self.transport=my_transport_lib.TranSport_new()

    def open(self,path):
        return my_transport_lib.TranSport_open_(self.transport,path)

    def getInputReport(self,lenth):
        return my_transport_lib.TranSport_getInputReport(self.transport,lenth)
        
    def read(self):
        result_ptr = my_transport_lib.TranSport_read(self.transport)
        
        # 如果返回的指针有效
        if result_ptr:
            # 假设最大长度为 512 字节
            MAX_LENGTH = 512
            result_bytes = ctypes.string_at(result_ptr, MAX_LENGTH)  # 读取指定长度的数据
            if len(result_bytes) > 0:
                # print(f"Raw received bytes: {result_bytes}")
                # 提取 ACK 和 OK 部分
                ack_response = result_bytes[:3].decode('utf-8', errors='ignore')  # 提取 ACK
                ok_response = result_bytes[5:7].decode('utf-8', errors='ignore')  # 提取 OK
                # print(f"Acknowledgement: {ack_response}, OK: {ok_response}")
                # 提取按键和状态部分
                key = result_bytes[9]  # 按键（第8个字节）
                status = result_bytes[10]  # 按键状态（第9个字节）
                # print(f"Key: {key}, Status: {status}")
        #     else:
        #         print("Received empty data.")
        # else:
        #     print("Read failed or returned NULL.")
        return result_bytes

    def wirte(self,data,lenth):
        return my_transport_lib.TranSport_write(self.transport,data,lenth)
    
    def freeEnumerate(self,devs):
        my_transport_lib.TranSport_freeEnumerate(self.transport,devs)

    def enumerate(self,vid,pid):
        
        vendor_id = vid or 0
        product_id = pid or 0
        device_list = []
        device_enumeration = my_transport_lib.TranSport_enumerate(self.transport,vendor_id,product_id)
        if device_enumeration:
            current_device = device_enumeration
            while current_device:
                device_list.append({
                    'path': current_device.contents.path.decode('utf-8'),
                    'vendor_id': current_device.contents.vendor_id,
                    'product_id': current_device.contents.product_id,
                })
                current_device = current_device.contents.next
        self.freeEnumerate(device_enumeration)
        return device_list


    def setBrightness(self,percent):
        return my_transport_lib.TranSport_setBrightness(self.transport,percent)
    
    def setBackgroundImg(self,buffer,size):
        return my_transport_lib.TranSport_setBackgroundImg(self.transport,buffer,size)
    
    def setKeyImg(self,path,key):
        return my_transport_lib.TranSport_setKeyImg(self.transport,path,key)
        
    # def setKeyImgData(self,imagedata,key,width,height):
    #     return my_transport_lib.TranSport_setKeyImgData(self.transport,imagedata,key,width,height)

    def setBackgroundImgDualDevice(self, path):
        return my_transport_lib.TranSport_setBackgroundImgDualDevice(self.transport, path)
    
    def setKeyImgDualDevice(self, path, key):
        return my_transport_lib.TranSport_setKeyImgDualDevice(self.transport, path, key)

    def setKeyImgDataDualDevice(self, path, key):
        return my_transport_lib.TranSport_setKeyImgDataDualDevice(self.transport, path, key)
    
    def keyClear(self,index):
        return my_transport_lib.TranSport_keyClear(self.transport,index)
    
    def keyAllClear(self):
        return my_transport_lib.TranSport_keyAllClear(self.transport)
    
    def wakeScreen(self):
        return my_transport_lib.TranSport_wakeScreen(self.transport)
    
    def refresh(self):
        return my_transport_lib.TranSport_refresh(self.transport)
    
    def disconnected(self):
        return my_transport_lib.TranSport_disconnected(self.transport)
    
    def close(self):
        return my_transport_lib.TranSport_close(self.transport)
    
    # def screen_Off(self):
    #     return my_transport_lib.TranSport_screenOff(self.transport)
    # def screen_On(self):
    #     return my_transport_lib.TranSport_screenOn(self.transport)