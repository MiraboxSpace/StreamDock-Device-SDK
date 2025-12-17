import platform
import threading
import time
from typing import Optional
# import pywinusb.hid as hid
from .ProductIDs import USBVendorIDs, USBProductIDs, g_products
from .Transport.LibUSBHIDAPI import LibUSBHIDAPI

# Platform-specific imports
if platform.system() == "Linux":
    import pyudev
elif platform.system() == "Windows":
    try:
        import wmi
        import pythoncom
        WINDOWS_SUPPORT = True
    except ImportError:
        print("警告: wmi 模块未安装，将使用轮询模式")
        WINDOWS_SUPPORT = False
elif platform.system() == "Darwin":
    # macOS specific imports can be added here if needed
    pass

class DeviceManager:
    streamdocks = list()

    @staticmethod
    def _get_transport(transport):
        return LibUSBHIDAPI()

    def __init__(self, transport=None):
        self.transport = self._get_transport(transport)

    def enumerate(self)->list:
        # CRITICAL: Clear old list to avoid stale references
        self.streamdocks.clear()
        
        products = g_products
        for vid, pid, class_type in products:
            found_devices = self.transport.enumerate_devices(vendor_id = vid, product_id = pid)
            # 为每个设备创建独立的 LibUSBHIDAPI 实例
            # CRITICAL: Pass device info to transport for proper resource management
            for d in found_devices:
                # Create device_info structure from dict
                device_info = LibUSBHIDAPI.create_device_info_from_dict(d)
                device_transport = LibUSBHIDAPI(device_info)
                self.streamdocks.append(class_type(device_transport, d))
        return self.streamdocks

    def listen(self):
        """
        监听设备插拔事件，支持跨平台
        """
        products = g_products
        system = platform.system()
        
        if system == "Linux":
            self._listen_linux(products)
        elif system == "Windows":
            self._listen_windows(products)
        elif system == "Darwin":
            self._listen_macos(products)
        else:
            print(f"不支持的操作系统: {system}")
    
    def _listen_linux(self, products):
        """Linux 系统使用 pyudev 监听设备事件"""
        context = pyudev.Context()
        monitor = pyudev.Monitor.from_netlink(context)
        monitor.filter_by(subsystem='usb')

        for device in iter(monitor.poll, None):
            self._handle_device_event(device.action, device, products)
    
    def _listen_windows(self, products):
        """Windows 系统使用 WMI 监听设备事件"""
        if not WINDOWS_SUPPORT:
            print("WMI 不可用，使用轮询模式")
            self._fallback_polling(products)
            return
            
        try:
            pythoncom.CoInitialize()
            c = wmi.WMI()
            
            # 监听设备连接事件
            watcher = c.Win32_DeviceChangeEvent.watch_for(
                EventType=2  # 设备连接
            )
            
            while True:
                try:
                    event = watcher()
                    if event.EventType == 2:  # 设备连接
                        self._check_new_devices_windows(products)
                    elif event.EventType == 3:  # 设备断开
                        self._check_removed_devices_windows(products)
                except Exception as e:
                    print(f"Windows 设备监听错误: {e}")
                    time.sleep(1)
        except Exception as e:
            print(f"Windows WMI 初始化失败: {e}")
            # 降级到轮询模式
            self._fallback_polling(products)
        finally:
            pythoncom.CoUninitialize()
    
    def _listen_macos(self, products):
        """macOS 系统使用轮询模式监听设备事件"""
        self._fallback_polling(products)
    
    def _fallback_polling(self, products):
        """降级到轮询模式，适用于不支持实时监听的系统"""
        # print("使用轮询模式监听设备变化...")
        current_devices = set()
        
        # 初始化当前设备列表
        for vid, pid, _ in products:
            devices = self.transport.enumerate_devices(vendor_id=vid, product_id=pid)
            for device in devices:
                current_devices.add(device['path'])
        
        while True:
            try:
                new_devices = set()
                for vid, pid, _ in products:
                    devices = self.transport.enumerate_devices(vendor_id=vid, product_id=pid)
                    for device in devices:
                        new_devices.add(device['path'])
                
                # 检查新增设备
                added_devices = new_devices - current_devices
                for device_path in added_devices:
                    print(f"[add] path: {device_path}")
                    self._handle_device_addition(device_path, products)
                
                # 检查移除设备
                removed_devices = current_devices - new_devices
                for device_path in removed_devices:
                    print(f"[remove] path: {device_path}")
                    self._handle_device_removal(device_path)
                
                current_devices = new_devices
                time.sleep(2)  # 每2秒检查一次
            except Exception as e:
                print(f"轮询监听错误: {e}")
                time.sleep(5)
    
    def _handle_device_event(self, action, device, products):
        """处理设备事件（Linux）"""
        if action not in ['add', 'remove']:
            return
            
        if action == 'remove':
            for willRemoveDevice in self.streamdocks:
                if device.device_path.find(willRemoveDevice.getPath()) != -1:
                    print("[remove] path: " + willRemoveDevice.getPath())
                    self.streamdocks.remove(willRemoveDevice)
                    break
        
        vendor_id_str = device.get('ID_VENDOR_ID')
        product_id_str = device.get('ID_MODEL_ID')

        if not vendor_id_str or not product_id_str:
            return

        try:
            vendor_id = int(vendor_id_str, 16)
            product_id = int(product_id_str, 16)
        except ValueError:
            return

        for vid, pid, class_type in products:
            if vendor_id == vid and product_id == pid:
                if action == 'add':
                    dev_path = device.device_path.split('/')[-1] + ":1.0"
                    full_path = dev_path

                    found_devices = self.transport.enumerate_devices(vendor_id=vid, product_id=pid)
                    for d in found_devices:
                        if d['path'].endswith(full_path):
                            print("[add] path:", d['path'])
                            newDevice = class_type(self.transport, d)
                            self.streamdocks.append(newDevice)
                            newDevice.open()
                            # your reconnect logic like the next two line
                            # newDevice.set_key_image(1, "../img/tiga64.png")
                            # newDevice.refresh()
                            break
    
    def _check_new_devices_windows(self, products):
        """检查 Windows 系统上的新设备"""
        for vid, pid, class_type in products:
            found_devices = self.transport.enumerate_devices(vendor_id=vid, product_id=pid)
            for device_info in found_devices:
                device_path = device_info['path']
                # 检查设备是否已存在
                exists = any(device.getPath() == device_path for device in self.streamdocks)
                if not exists:
                    print(f"[add] path: {device_path}")
                    newDevice = class_type(self.transport, device_info)
                    self.streamdocks.append(newDevice)
                    newDevice.open()
                    # your reconnect logic like the next two line
                    # newDevice.set_key_image(1, "../img/tiga64.png")
                    # newDevice.refresh()
    
    def _check_removed_devices_windows(self, products):
        """检查 Windows 系统上已移除的设备"""
        current_paths = set()
        for vid, pid, _ in products:
            found_devices = self.transport.enumerate_devices(vendor_id=vid, product_id=pid)
            for device_info in found_devices:
                current_paths.add(device_info['path'])
        
        # 移除不再存在的设备
        devices_to_remove = []
        for device in self.streamdocks:
            if device.getPath() not in current_paths:
                devices_to_remove.append(device)
        
        for device in devices_to_remove:
            print(f"[remove] path: {device.getPath()}")
            self.streamdocks.remove(device)
    
    def _handle_device_addition(self, device_path, products):
        """处理设备添加事件（轮询模式）"""
        for vid, pid, class_type in products:
            found_devices = self.transport.enumerate_devices(vendor_id=vid, product_id=pid)
            for device_info in found_devices:
                if device_info['path'] == device_path:
                    newDevice = class_type(self.transport, device_info)
                    self.streamdocks.append(newDevice)
                    newDevice.open()
                    # your reconnect logic like the next two line
                    # newDevice.set_key_image(1, "../img/tiga64.png")
                    # newDevice.refresh()
                    break
    
    def _handle_device_removal(self, device_path):
        """处理设备移除事件（轮询模式）"""
        devices_to_remove = []
        for device in self.streamdocks:
            if device.getPath() == device_path:
                devices_to_remove.append(device)
        
        for device in devices_to_remove:
            self.streamdocks.remove(device)

