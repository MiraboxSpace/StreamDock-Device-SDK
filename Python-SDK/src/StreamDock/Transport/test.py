from LibUSBHIDAPI import LibUSBHIDAPI
usb_hid = LibUSBHIDAPI()

# vendor_id = 0x5548 
# product_id = 0x1008


vendor_id = 0x6603
product_id = 0x1009
# Get device list
device_list = usb_hid.enumerate_devices(vendor_id, product_id)
print(f"Found {len(device_list)} device(s).")
# # Print device info
# for device in device_list:
#     print(f"Device Path: {device['path']}")
#     print(f"Vendor ID: {device['vendor_id']}")  
#     print(f"Product ID: {device['product_id']}")
#     print("----")
