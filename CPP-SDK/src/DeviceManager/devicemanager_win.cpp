#include "devicemanager.h"
#ifdef _WIN32

struct WindowContext
{
	DeviceManager* manager{ nullptr };
	std::function<void(std::shared_ptr<StreamDock>)> connectCallback{ nullptr };
	// you can add some var to send it to the windows
	// and convert it to your var by
	// auto xxx = ctx->xxx;
};

LRESULT DeviceManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DEVICECHANGE)
	{
		WindowContext* ctx = reinterpret_cast<WindowContext*>(GetProp(hwnd, TEXT("@thiPtr_DeviceManager&autoReconnect@")));
		auto manager = ctx->manager;
		auto& connectCallback = ctx->connectCallback;
		PDEV_BROADCAST_DEVICEINTERFACE pDevIntf;
		switch (wParam)
		{
		case DBT_DEVICEARRIVAL:
		{
			pDevIntf = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
			if (pDevIntf->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
			{
				std::string devicePath = pDevIntf->dbcc_name;
				/// if the device support begin
				bool canConnect = false;
				hid_device_info* devs = hid_enumerate(0x0, 0x0);
				auto cur = devs;
				while (cur)
				{
					DeviceEnumerator::DeviceInfo device(*cur);
					if (StreamDockFactory::instance().exist(device._vendor_id, device._product_id) && StreamDock::isStreamDockHidDeviceUsage(device.toPureHidDeviceInfo()) /* &&
						 StreamDock::isStreamDockHidDevice(device.toPureHidDeviceInfo())*/
						&& (cur->path == devicePath))
						canConnect = true;
					cur = cur->next;
				}
				if (!canConnect) 
				{
					hid_free_enumeration(devs);
					break;
				}
				hid_free_enumeration(devs);
				/// if the device support end
				manager->enumerator();
				if (!connectCallback)
					break;
				std::shared_ptr<StreamDock> dock;
				{
					std::lock_guard lock(manager->streamdocksMutex_);
					dock = (manager->getStreamDocks().find(devicePath))->second;
				}
				ToolKit::print("[+] HID Device Added: ", devicePath);
				try
				{
					connectCallback(dock);
				}
				catch (const std::exception& e)
				{
					ToolKit::print("[ERROR] connectCallback threw exception: ", e.what());
				}
			}
			break;
		}
		case DBT_DEVICEREMOVECOMPLETE:
		{ // remove
			pDevIntf = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
			if (pDevIntf->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
				break;
			std::string devicePath = pDevIntf->dbcc_name;
			std::lock_guard lock(manager->streamdocksMutex_);
			auto found = manager->getStreamDocks().find(devicePath);
			if (found != manager->getStreamDocks().end())
			{
				manager->getStreamDocks().erase(devicePath);
				ToolKit::print("[-] HID Device Removed: ", devicePath);
			}
			break;
		}
		default:
			break;
		}
	}
	else if (uMsg == WM_CLOSE)
	{
		auto ctx = reinterpret_cast<WindowContext*>(GetProp(hwnd, TEXT("@thiPtr_DeviceManager&autoReconnect@")));
		if (ctx)
		{
			RemoveProp(hwnd, TEXT("@thiPtr_DeviceManager&autoReconnect@"));
			delete ctx;
		}
		PostQuitMessage(0);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void DeviceManager::listen(std::function<void(std::shared_ptr<StreamDock>)> connect_and_run)
{
	auto listener = [this, connect_and_run]
		{
			bool use_hid = true;
			const GUID GUID_DEVINTERFACE_HID = { 0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };		  /// hid guid
			const GUID GUID_DEVINTERFACE_USB_DEVICE = { 0xA5DCBF10, 0x6530, 0x11D2, {0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED} }; /// usb guid
			HDEVNOTIFY hDeviceNotify = nullptr;

			WNDCLASS wc = { 0 };
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = GetModuleHandle(nullptr);
			wc.lpszClassName = TEXT("USBDeviceListener");
			ATOM result = RegisterClass(&wc);
			if (!result && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
			{
				ToolKit::print("[ERROR] RegisterClass failed");
				return;
			}

			hwnd_ = CreateWindowEx(0, TEXT("USBDeviceListener"), TEXT("USB Listener"),
				WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				nullptr, nullptr, wc.hInstance, nullptr);
			auto context = new WindowContext{ this, connect_and_run };
			SetProp(hwnd_, TEXT("@thiPtr_DeviceManager&autoReconnect@"), context);

			if (!hwnd_)
				return;
			// RegisterForDeviceNotifications
			DEV_BROADCAST_DEVICEINTERFACE NotificationFilter = {};
			NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
			NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
			NotificationFilter.dbcc_classguid = use_hid ? GUID_DEVINTERFACE_HID : GUID_DEVINTERFACE_USB_DEVICE;
			hDeviceNotify = RegisterDeviceNotification(hwnd_, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
			if (!hDeviceNotify)
			{
				if (hwnd_)
					DestroyWindow(hwnd_);
				return;
			}

			MSG msg;
			isListening_ = true;
			while (isListening_ && GetMessage(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (hDeviceNotify)
				UnregisterDeviceNotification(hDeviceNotify);
			if (hwnd_)
				DestroyWindow(hwnd_);
			return;
		};
	listener_ = std::thread([this, listener]
		{ listener(); });
}

void DeviceManager::stopListen()
{
	isListening_ = false;
	if (hwnd_)
	{
		PostMessage(hwnd_, WM_CLOSE, 0, 0);
	}
	if (listener_.joinable())
	{
		listener_.join();
	}
}

#endif