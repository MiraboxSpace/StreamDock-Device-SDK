from LibUSBHIDAPI import LibUSBHIDAPI
usb_hid = LibUSBHIDAPI()

vendor_id = 26115 
product_id = 4101

# 获取设备列表
device_list = usb_hid.enumerate(vendor_id, product_id)

# 打印设备信息
for device in device_list:
    print(f"Device Path: {device['path']}")
    print(f"Vendor ID: {device['vendor_id']}")  
    print(f"Product ID: {device['product_id']}")
    print("----")
