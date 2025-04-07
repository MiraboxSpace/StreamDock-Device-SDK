#include "DeviceManager.h"
#include "ProductIDs.h"
#include <iostream>
#include <string>
#include <regex>

#ifdef _WIN32
#include <windows.h>
#include <setupapi.h>
#include <windows.h>
#include <Dbt.h>
#include <set>
#pragma comment(lib, "Setupapi.lib") // link SetupAPI library
#elif defined(__linux__)
#include <libudev.h>
#include <thread>
#include <unistd.h>
#include <cstring>
#include <codecvt>
#include <locale>
#elif defined(__APPLE__)
#endif

DeviceManager::DeviceManager()
{
	this->transport.reset(new TranSport());
}

DeviceManager::~DeviceManager()
{
	clearListen();
	if (m_future.valid())
		m_future.get();
}

std::map<std::string, std::shared_ptr<StreamDock>> DeviceManager::enumerate()
{
	for (auto device : StreamDockDevices)
	{
		// Check if there are devices with the specified VID and PID, and return a linked list if found
		auto infoList = this->transport->enumerateEx(device.first.first, device.first.second);
		auto deviceInfo = infoList._info;
		while (deviceInfo)
		{
			// add device to map
			if (deviceInfo->interface_number == 0)
			{
				std::cout << "----------------------------------------" << std::endl;
				std::cout << "Path:" << deviceInfo->path << std::endl;
				std::cout << "Vendor ID:" << deviceInfo->vendor_id << std::endl;
				std::cout << "Product ID:" << deviceInfo->product_id << std::endl;
				std::wcout << "Serial Number:" << deviceInfo->serial_number << std::endl;
				std::wcout << "Manufacturer:" << deviceInfo->manufacturer_string << std::endl;
				std::wcout << "Product:" << deviceInfo->product_string << std::endl;
				std::cout << "Usage:" << deviceInfo->usage << std::endl;
				std::cout << "Usage Page:" << deviceInfo->usage_page << std::endl;
				std::cout << "Interface Number:" << deviceInfo->interface_number << std::endl;
				std::cout << "Release Number:" << deviceInfo->release_number << std::endl;
				std::cout << "----------------------------------------" << std::endl;
				addToPathMap(deviceInfo);
				switch (device.second)
				{
				case Device293:
				{
					m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDock293(this->transport, deviceInfo));
					break;
				}
				case Device293s:
				{
					// m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDock293s(this->transport, deviceInfo));
					break;
				}
				case Device293V3:
				{
					m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDock293V3(this->transport, deviceInfo));
					break;
				}
				case Device293N3V25:
				{
					m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDockN3(this->transport, deviceInfo));
					break;
				}
				case Device293N4:
				{
					m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDockN4(this->transport, deviceInfo));
					break;
				}
				case Device293N1:
				{
					m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDockN1(this->transport, deviceInfo));
					break;
				}
				default:
				{
					break;
				}
				}
			}
			deviceInfo = deviceInfo->next;
		}
	}
	return m_deviceMap;
}

#ifdef _WIN32
int DeviceManager::listen(bool autoReconnect)
{
	struct WindowContext {
		bool autoReconnect{ false };
		DeviceManager* manager{ nullptr };
		// you can add some var to send it to the windows
		// and convert it to your var by
		// auto xxx = ctx->xxx;
	};
	bool use_hid = true;
	// hid guid
	const GUID GUID_DEVINTERFACE_HID = { 0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };
	// usb guid
	const GUID GUID_DEVINTERFACE_USB_DEVICE = { 0xA5DCBF10, 0x6530, 0x11D2, {0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED} };
	HDEVNOTIFY hDeviceNotify = nullptr;
	auto WindowProc = [](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
		{
			if (uMsg == WM_DEVICECHANGE)
			{
				WindowContext* ctx = reinterpret_cast<WindowContext*>(GetProp(hwnd, TEXT("@thiPtr_DeviceManager&autoReconnect@")));
				auto manager = ctx->manager;
				auto autoReconnect = ctx->autoReconnect;
				PDEV_BROADCAST_DEVICEINTERFACE pDevIntf;
				switch (wParam)
				{
				case DBT_DEVICEARRIVAL:
				{ // plugn
					pDevIntf = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
					if (pDevIntf->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
					{
						// Detect and confirm insertion of a USB/HID device
						std::string devicePath = pDevIntf->dbcc_name;
						/*
						better method: no hid_device_info
						std::regex vidPidRegex(R"(vid_([0-9A-Fa-f]{4})&pid_([0-9A-Fa-f]{4}))", std::regex_constants::icase);
						std::smatch match;
						int vid = 0;
						int pid = 0;
						if (std::regex_search(devicePath, match, vidPidRegex) && match.size() == 3) {
							int vid = std::stoi(match[1], nullptr, 16);
							int pid = std::stoi(match[2], nullptr, 16);
						}
						if (manager->m_deviceMap.find(devicePath) == manager->m_deviceMap.end())
						{
							// Determine the type of the detected device
							auto _pvPair = std::pair<USBVendorIDs, USBProductIDs>(static_cast<USBVendorIDs>(vid), static_cast<USBProductIDs>(pid));
							auto found = StreamDockDevices.find(_pvPair);
							if (found == StreamDockDevices.end()) break;
							auto deviceType = found->second;
							switch (deviceType)
							{
							case Device293:
							{
								manager->m_deviceMap[std::string(devicePath)] = std::shared_ptr<StreamDock>(new StreamDock293(manager->transport, nullptr));
								break;
							}
							case Device293s:
							{
								//manager->m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDock293s(this->transport, deviceInfo));
								break;
							}
							case Device293V3:
							{
								manager->m_deviceMap[std::string(devicePath)] = std::shared_ptr<StreamDock>(new StreamDock293V3(manager->transport, nullptr));
								break;
							}
							case Device293N3:
							{
								manager->m_deviceMap[std::string(devicePath)] = std::shared_ptr<StreamDock>(new StreamDockN3(manager->transport, nullptr));
								break;
							}
							case Device293N4:
							{
								manager->m_deviceMap[std::string(devicePath)] = std::shared_ptr<StreamDock>(new StreamDockN4(manager->transport, nullptr));
								break;
							}
							default:
							{
								break;
							}
							}
						}
						*/
						// remove the same
						for (auto& device : StreamDockDevices)
						{
							// Check if there are devices with the specified VID and PID, and return a linked list if found
							auto infoList = manager->transport->enumerateEx(device.first.first, device.first.second);
							auto deviceInfo = infoList._info;
							while (deviceInfo)
							{
								// add device to map
								if (deviceInfo->interface_number == 0 && manager->m_deviceMap.find(deviceInfo->path) == manager->m_deviceMap.end())
								{
									std::cout << "----------------------------------------" << std::endl;
									std::cout << "[+] Device Inserted: " << deviceInfo->path << std::endl;
									std::cout << "Path:" << deviceInfo->path << std::endl;
									std::cout << "Vendor ID:" << deviceInfo->vendor_id << std::endl;
									std::cout << "Product ID:" << deviceInfo->product_id << std::endl;
									std::wcout << "Serial Number:" << deviceInfo->serial_number << std::endl;
									std::wcout << "Manufacturer:" << deviceInfo->manufacturer_string << std::endl;
									std::wcout << "Product:" << deviceInfo->product_string << std::endl;
									std::cout << "Usage:" << deviceInfo->usage << std::endl;
									std::cout << "Usage Page:" << deviceInfo->usage_page << std::endl;
									std::cout << "Interface Number:" << deviceInfo->interface_number << std::endl;
									std::cout << "Release Number:" << deviceInfo->release_number << std::endl;
									std::cout << "----------------------------------------" << std::endl;
									switch (device.second)
									{
									case Device293:
									{
										manager->m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDock293(manager->transport, deviceInfo));
										break;
									}
									case Device293s:
									{
										// manager->m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDock293s(manager->transport, deviceInfo));
										break;
									}
									case Device293V3:
									{
										manager->m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDock293V3(manager->transport, deviceInfo));
										break;
									}
									case Device293N3V25:
									{
										manager->m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDockN3(manager->transport, deviceInfo));
										break;
									}
									case Device293N4:
									{
										manager->m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDockN4(manager->transport, deviceInfo));
										break;
									}
									case Device293N1:
									{
										manager->m_deviceMap[std::string(deviceInfo->path)] = std::shared_ptr<StreamDock>(new StreamDockN1(manager->transport, deviceInfo));
										break;
									}
									default:
									{
										break;
									}
									}
									if (autoReconnect && manager->m_deviceMap.find(deviceInfo->path) != manager->m_deviceMap.end())
									{
										// create success and reconnect
										manager->m_deviceMap[deviceInfo->path]->open();
										manager->m_deviceMap[deviceInfo->path]->wakeScreen();
										// reconnect and do something here, like launch a singal to call a function
									}
								}
								deviceInfo = deviceInfo->next;
							}
						}
					}
					break;
				}
				case DBT_DEVICEREMOVECOMPLETE:
				{ // remove
					pDevIntf = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
					if (pDevIntf->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
					{
						// Detect and confirm insertion of a USB/HID device
						std::string devicePath = pDevIntf->dbcc_name;
						auto found = manager->m_deviceMap.find(devicePath);
						if (found != manager->m_deviceMap.end())
						{
							std::cout << "[-] Device Removed: " << devicePath << std::endl;
							manager->m_deviceMap.erase(devicePath);
						}
					}
					break;
				}
				default:
				{
					break;
				}
				}
			}
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		};
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = TEXT("USBDeviceListener");
	if (!RegisterClass(&wc))
		return 0;

	// HWND hwnd = CreateWindow(TEXT("USBDeviceListener"), TEXT("USB Listener"), WS_OVERLAPPEDWINDOW,
	//	CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	//	nullptr, nullptr, wc.hInstance, this);
	//	nullptr, nullptr, wc.hInstance, this);
	// Pass the current object (`this` pointer) to the `CreateWindowEx` function
	HWND hwnd = CreateWindowEx(0, TEXT("USBDeviceListener"), TEXT("USB Listener"),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr, nullptr, wc.hInstance, nullptr);
	auto context = new WindowContext{ autoReconnect, this };
	SetProp(hwnd, TEXT("@thiPtr_DeviceManager&autoReconnect@"), context);

	if (!hwnd)
		return 0;

	// RegisterForDeviceNotifications
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter = {};
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	NotificationFilter.dbcc_classguid = use_hid ? GUID_DEVINTERFACE_HID : GUID_DEVINTERFACE_USB_DEVICE;
	hDeviceNotify = RegisterDeviceNotification(hwnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
	if (!hDeviceNotify)
	{
		if (hwnd)
			DestroyWindow(hwnd);
		return 0;
	}

	MSG msg;
	isListening = true;
	while (isListening && GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (hDeviceNotify)
		UnregisterDeviceNotification(hDeviceNotify);
	if (hwnd)
		DestroyWindow(hwnd);
	return 1;
}
#elif defined(__linux__)
int DeviceManager::listen(bool autoReconnect)
{
	isListening = true;
	struct udev* udev = udev_new();
	if (!udev)
	{
		std::cerr << "Cannot create udev context." << std::endl;
		return 0;
	}

	struct udev_monitor* mon = udev_monitor_new_from_netlink(udev, "udev");
	if (!mon)
	{
		std::cerr << "Cannot create udev monitor." << std::endl;
		udev_unref(udev);
		return 0;
	}
	udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_device");
	// udev_monitor_filter_add_match_subsystem_devtype(mon, "hidraw", nullptr);
	udev_monitor_enable_receiving(mon);
	int fd = udev_monitor_get_fd(mon);
	std::cout << "Start listening for HID device plug/unplug events..." << std::endl;
	while (isListening)
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		int ret = select(fd + 1, &fds, nullptr, nullptr, nullptr);
		if (ret > 0 && FD_ISSET(fd, &fds))
		{
			struct udev_device* dev = udev_monitor_receive_device(mon);
			if (dev)
			{
				const char* action = udev_device_get_action(dev);
				const char* devnode = udev_device_get_devnode(dev);
				std::string act = action ? action : "";
				if (!act.empty() && devnode)
				{
					std::string nodeStr(devnode);
					if (act == "add")
					{
						for (auto device : StreamDockDevices)
						{
							auto infoList = this->transport->enumerateEx(device.first.first, device.first.second);
							auto deviceInfo = infoList._info;
							while (deviceInfo)
							{
								if (deviceInfo->interface_number == 0)
								{
									std::string hidapi_path = deviceInfo->path;
									// create devnode -> hidapi_path mapping
									this->m_devnodeToPath[nodeStr] = hidapi_path;
									std::cout << "----------------------------------------" << std::endl;
									std::cout << "[" << act << "] libusb: " << devnode << "; hid_path: " << m_devnodeToPath[devnode] << std::endl;
									std::cout << "Path:" << deviceInfo->path << std::endl;
									std::cout << "Vendor ID:" << deviceInfo->vendor_id << std::endl;
									std::cout << "Product ID:" << deviceInfo->product_id << std::endl;
									std::wcout << "Serial Number:" << deviceInfo->serial_number << std::endl;
									std::wcout << "Manufacturer:" << deviceInfo->manufacturer_string << std::endl;
									std::wcout << "Product:" << deviceInfo->product_string << std::endl;
									std::cout << "Usage:" << deviceInfo->usage << std::endl;
									std::cout << "Usage Page:" << deviceInfo->usage_page << std::endl;
									std::cout << "Interface Number:" << deviceInfo->interface_number << std::endl;
									std::cout << "Release Number:" << deviceInfo->release_number << std::endl;
									std::cout << "----------------------------------------" << std::endl;
									if (std::string(deviceInfo->path) == hidapi_path)
									{
										switch (device.second)
										{
										case Device293:
											m_deviceMap[hidapi_path] = std::make_shared<StreamDock293>(this->transport, deviceInfo);
											break;
										case Device293s:
											// m_deviceMap[hidapi_path] = std::make_shared<StreamDock293s>(this->transport, deviceInfo);
											break;
										case Device293V3:
											m_deviceMap[hidapi_path] = std::make_shared<StreamDock293V3>(this->transport, deviceInfo);
											break;
										case Device293N3V25:
											m_deviceMap[hidapi_path] = std::make_shared<StreamDockN3>(this->transport, deviceInfo);
											break;
										case Device293N4:
											m_deviceMap[hidapi_path] = std::make_shared<StreamDockN4>(this->transport, deviceInfo);
											break;
										case Device293N1:
											m_deviceMap[hidapi_path] = std::make_shared<StreamDockN1>(this->transport, deviceInfo);
											break;
										default:
											break;
										}
										if (autoReconnect && m_deviceMap.find(hidapi_path) != m_deviceMap.end())
										{
											// create success and reconnect
											m_deviceMap[hidapi_path]->open();
											m_deviceMap[hidapi_path]->wakeScreen();
											// reconnect and do something here, like launch a singal to call a function
										}
									}
								}
								deviceInfo = deviceInfo->next;
							}
						}
					}
					else if (act == "remove")
					{
						// remove device: use devnode find path
						auto it = this->m_devnodeToPath.find(nodeStr);
						if (it != this->m_devnodeToPath.end())
						{
							std::cout << "[" << act << "] libusb: " << devnode << "; hid_path: " << m_devnodeToPath[devnode] << std::endl;
							const std::string& path = it->second;
							m_deviceMap.erase(path);
							this->m_devnodeToPath.erase(it);
						}
					}
				}
				udev_device_unref(dev);
			}
		}
	}
	udev_monitor_unref(mon);
	udev_unref(udev);
	return 1;
}
#elif defined(__APPLE__)
#endif

void DeviceManager::clearListen()
{
	isListening = false;
}

void DeviceManager::addToPathMap(hid_device_info* info)
{
#ifdef _WIN32
	// pass
#elif defined(__linux__)
	auto wstring_to_utf8 = [](const std::wstring& wstr) -> std::string
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
			return conv.to_bytes(wstr);
		};
	auto get_devnode_from_hid_info = [wstring_to_utf8](const hid_device_info* info) -> std::string
		{
			if (!info)
				return "";

			struct udev* udev = udev_new();
			if (!udev)
				return "";

			struct udev_enumerate* enumerate = udev_enumerate_new(udev);
			udev_enumerate_add_match_subsystem(enumerate, "usb");
			udev_enumerate_add_match_property(enumerate, "DEVTYPE", "usb_device");
			udev_enumerate_scan_devices(enumerate);

			struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
			struct udev_list_entry* entry;

			udev_list_entry_foreach(entry, devices)
			{
				const char* path = udev_list_entry_get_name(entry);
				struct udev_device* dev = udev_device_new_from_syspath(udev, path);
				if (!dev)
					continue;

				const char* vid = udev_device_get_sysattr_value(dev, "idVendor");
				const char* pid = udev_device_get_sysattr_value(dev, "idProduct");
				const char* serial = udev_device_get_sysattr_value(dev, "serial");

				if (vid && pid)
				{
					int vid_val = std::stoi(vid, nullptr, 16);
					int pid_val = std::stoi(pid, nullptr, 16);
					bool serial_match = true;

					if (info->serial_number && serial)
					{
						std::string serial_utf8 = wstring_to_utf8(info->serial_number);
						serial_match = (serial_utf8 == serial);
					}
					if (vid_val == info->vendor_id && pid_val == info->product_id && serial_match)
					{
						const char* devnode = udev_device_get_devnode(dev); // /dev/bus/usb/...
						std::string result = devnode ? devnode : "";
						udev_device_unref(dev);
						udev_enumerate_unref(enumerate);
						udev_unref(udev);
						return result;
					}
				}
				udev_device_unref(dev);
			}
			udev_enumerate_unref(enumerate);
			udev_unref(udev);
			return "";
		};
	auto devnode = get_devnode_from_hid_info(info);
	m_devnodeToPath[devnode] = info->path;
#elif defined(__APPLE__)
	// pass
#endif
}

void DeviceManager::asyncListen()
{
	if (!m_future.valid())
	{
		m_future =
			std::async(std::launch::async, [this]()
				{ listen(AUTORECONNECT); });
	}
}
