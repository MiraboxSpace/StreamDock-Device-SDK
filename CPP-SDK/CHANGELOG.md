
# 更新日志 / Changelog

## Version 1.0.1 - 2025-03-29 (Sat)  
**作者 / Author:** xiaochufuji  

### 新增 / Added
- 新增设备支持：**N1**。  
  Added support for **N1** device.
- N1 支持接口 `switchMode`，可在 **计算器 / 键盘 / 上位机模式** 之间切换。  
  N1 supports the `switchMode` interface, allowing switching between **calculator**, **keyboard**, and **host** modes.
- 副屏图片支持自动协商为设备规定的图片格式。  
  Secondary screen image now automatically negotiates to the format required by the N1 device.
- 新增 `joining_thread`：作用域内自动 join 的线程类，适用于监听设备插拔（`listen`）或读取设备信息（`read`）的线程管理。  
  Introduced `joining_thread`: a scope-based thread wrapper that automatically joins the thread on destruction. Useful for device hotplug `listen` and device `read` operations.

### 依赖 / Dependencies
- C++17 及以上标准。  
  C++17 or above.
- [hidapi](https://github.com/libusb/hidapi) `v0.14.0`
- [libusb-1.0](https://libusb.info/) `v1.0.27`
- [libudev](https://github.com/systemd/systemd) `v245`（用于 Linux 设备监听）  
  (for Linux hotplug support)
- 设备支持模块需具备对 N1 模式切换功能的支持。  
  Device-level support required for N1 mode switching functionality.

---

## Version 1.0.0 - 2025-03-26 (Wed)  
**作者 / Author:** xiaochufuji  

### 初始版本 / Initial Release
- 跨平台 SDK 初始化发布。  
  Cross-platform SDK initialization.
- 初步支持设备：**N3、N4、293、293V3**。  
  Initial support for devices: **N3, N4, 293, 293V3**.
- 支持功能：  
  Supported features:
  - 设备插拔监听与自动重连。  
    Device hotplug detection and automatic reconnection.
  - 设置按键与背景图片。  
    Key mapping and background image configuration.
  - 监听设备返回信息（`read`）。  
    Listening for data from devices via `read`.
  - 图片自动协商为设备要求的格式。  
    Automatic image negotiation to match the format required by each device.

### 依赖 / Dependencies
- C++17 及以上标准。  
  C++17 or above.
- [hidapi](https://github.com/libusb/hidapi) `v0.14.0`
- [libusb-1.0](https://libusb.info/) `v1.0.27`
- [FreeImagePlus (FIP)](https://sourceforge.net/projects/freeimage/) `v3.18.0`（用于图像加载与处理）  
  (for image loading and conversion)
- [libudev](https://github.com/systemd/systemd) `v245`（仅限 Linux 系统）  
  (Linux only)
