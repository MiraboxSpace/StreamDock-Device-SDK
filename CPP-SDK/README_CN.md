# StreamDock C++ SDK 文档

## 1. 📘 本项目介绍

本项目是一个基于 C++17 开发的 SDK，旨在让用户能够通过代码直接控制 Stream Dock 设备。

该 SDK 提供了 Linux 与 Windows 的跨平台支持，用户可以根据自身的操作系统选择合适的构建方式进行编译和使用。目前 SDK 支持对部分设备的控制，例如：**293**、**293V3**、**N4**、**N3**、**N1** 等型号，用户可通过查看第五节的使用示例，快速上手并调用我们提供的接口对设备进行操作。

SDK 中可能依赖一些第三方库，例如：

- **OpenCV**：用于图像加载与处理；
- **hidapi**：用于 USB 设备的数据传输与控制；
- **giflib**：用于 处理动态图片 gif 的处理；
- 其他系统相关库（如 libudev）。

请根据自身平台及使用需求，参考第二节（Linux 构建）或第三节（Windows 构建），安装所需的依赖库并完成构建流程。

---

## 2. 🐧 Linux 构建

### 2.1 依赖安装

构建本 SDK 之前，请确保已安装以下系统依赖项：

#### 必需依赖项

- **CMake**：构建系统
- **libudev-dev**：用于监听 USB 设备插拔（仅限 Linux）
- **libusb-1.0-0-dev**：hidapi 的底层依赖（使用 libusb 版本）
- **libgif-dev**：gif 图像处理

#### 安装命令（适用于 Ubuntu / Debian）

```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install -y libudev-dev libusb-1.0-0-dev libgif-dev
```

除此之外，SDK 还需要你安装 OpenCV 库。

```bash
sudo apt install libopencv-dev
```

当然你可以使用裁剪过后的 `OpenCV`库，具体裁剪可以使用 `CMake-GUI`或命令行的方式进行裁剪，裁剪的目标是读写大部分的图片类型并支持他们进行编码至 `BGR888`、`BGR565`、`jpeg`和 `png`类型，确保 `SDK`正常运行。如果你觉得裁剪麻烦，请直接根据上述命令直接安装即可。

### 2.2 构建步骤

安装好依赖项后，进入项目主目录，你会看到如下结构：

```
├── bin
├── build
├── cmake
├── CMakeLists.txt
├── ImgProcesser
├── lib
├── src
└── third_party
```

你可以直接在命令行中执行以下命令进行构建：

```bash
chmod +x build.sh
./build.sh
```

当然你也可以使用自己的方式通过 CMake 构建，具体参数和配置可以查看 `build.sh` 脚本内容，自行构建。
⚠️ 构建期间如果出现找不到 `hid_get_input_report` 的错误，请参考第 [4.1](#41-%E7%BC%96%E8%AF%91%E6%97%B6%E6%8A%A5%E9%94%99undefined-reference-to-hid_get_input_report) 节进行处理。

### 2.3 运行示例

进入 `bin`目录， 然后使用管理员权限执行即可，如果此处你不希望使用sudo，就需要把你的设备添加到你的用户设备权限列表中，当然你可以修改cmake可执行文件输出的路径，但是这个时候你需要注意代码中自己修改的图片文件的相对路径

```
sudo ./main
```

**添加设备权限列表方法:**

把 99-streamdock.rules 复制到  /etc/udev/rules.d/ 目录.后执行:

```
sudo udevadm control --reload-rules
sudo udevadm trigger
```

---

## 3. 🪟 Windows 构建

### 3.1 环境设置

在 Windows 上，你不需要手动安装大多数第三方库。我们已经预编译了所需的 `.dll` 文件并将它们包含在 `bin` 目录中。

测试环境：

- 编译器：Visual Studio 17 2022
- 语言标准：C++17（也支持 C++20）

如果你使用的是 Visual Studio 2022 且支持 C++17 或更新版本，可以直接跳转到构建说明。

否则，请阅读本节以获取更多指导。

此 SDK 仅使用 **MSVC** 工具链进行了测试。我们建议使用 MSVC 以确保完全兼容。

在 CMake 中指定生成器：

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
```

### 💡 关于 C++ 标准

SDK 默认使用 C++17，因为使用了现代特性如 `std::filesystem`。

如果你更喜欢使用 **C++14** 或 **C++11**，可以修改代码。大多数需要的更改涉及将 `std::filesystem` 的使用替换为传统文件处理。这可以启用旧标准。

⚠️ 使用 MinGW 或其他编译器构建未经测试，不受官方支持。

---

### 3.2 使用 CMake 构建

你可以将整个项目文件夹（`bin/` 上面的那个）拖入 Visual Studio 2022。

Visual Studio 将自动检测并配置 CMake 项目。配置完成后，使用以下快捷键进行构建：

- `Ctrl + B`
- `Ctrl + Shift + B`

输出可执行文件将在 `bin` 目录下生成。

或者，你可以使用 Visual Studio 命令行进行构建：

> `x64 Native Tools Command Prompt for VS 2022`

在项目目录中运行以下命令：

```bat
build.bat
```

这也会生成可执行文件。

---

### 3.3 注意事项

我们已经预编译了所有必需的第三方库，并将它们包含在 SDK 的 `bin` 目录中：

- `opencv_core4120.dll`
- `opencv_imgcodecs4120.dll`
- `opencv_imgproc4120.dll`
- `hidapi.dll`

构建 SDK 后，只需将这些 `.dll` 文件复制到与可执行文件相同的目录中（例如 `bin/`），你就可以运行了 - 不需要额外安装。

---

## 4. 🍎 macOS 构建

### 4.1 依赖安装

在 macOS 上，SDK 包含为 macOS 预构建的 OpenCV 库。在构建之前，你需要准备 OpenCV 依赖项。

导航到 `ImgProcesser/third_party` 目录并运行以下脚本以创建 OpenCV 库的符号链接：

```bash
bash link_opencv_symlinks.sh
```

此脚本将在 `opencv/mac/lib` 中创建必要的符号链接，以允许 CMake 正确定位 OpenCV 库。

### 4.2 热插拔支持

macOS 版本通过轮询模式支持设备热插拔检测。`listen()` 方法会启动后台线程，每 2 秒自动检测设备的插入和拔出事件，与 Windows 和 Linux 平台功能一致。

### 4.3 构建步骤

准备好依赖项后，转到项目根目录。你将看到以下结构：

```
├── bin
├── build
├── cmake
├── CMakeLists.txt
├── ImgProcesser
├── lib
├── src
└── third_party
```

运行以下命令进行构建：

```bash
chmod +x build.sh
./build.sh
```

你也可以手动使用 CMake 构建。有关配置详情，请参考 `build.sh`。

## 5. ⚠️ 故障排除

### 5.1 编译时报错：undefined reference to `hid_get_input_report`

在 Linux 系统中，如果系统默认安装的 `hidapi` 库版本与本项目依赖不一致，可能会导致以下链接错误：

```text
/usr/bin/ld: /home/xxx/StreamDock_CppSDK/StreamDock_CppSDK/lib/libtransport.a: undefined reference to `hid_get_input_report'
collect2: error: ld returned 1 exit status
make[2]: *** [StreamDock_CppSDK/CMakeFiles/main.dir/build.make:197: /home/xxx/StreamDock_CppSDK/bin/main] Error 1
make[1]: *** [CMakeFiles/Makefile2:128: StreamDock_CppSDK/CMakeFiles/main.dir/all] Error 2
make: *** [Makefile:91: all] Error 2
```

解决方法:
先找到刚刚安装的libusb库

```
sudo find / -name libhidapi-libusb.so.0
```

假设说找到的路径为 `/usr/lib/x86_64-linux-gnu/libhidapi-libusb.so.0`，那么你只需要使用如下命令进行替换即可:

```
sudo cp ./lib/libhidapi-libusb.so.0 /usr/lib/x86_64-linux-gnu
```

### 5.2 运行时找不到设备 `find [0] device`

如果你在运行程序时看到如下提示：

```text
find [0] device
```

说明当前未能成功识别到任何连接的设备。

#### 🛠 可能原因及解决方法：

* **在 Linux 下运行时缺少权限**
  请检查你是否使用了 `sudo` 来运行程序：

```
sudo ./bin/main
```

* 没有管理员权限将导致设备无法被正确访问。
* **设备的 PID / VID 未在 SDK 中注册**
  请打开项目中的 `StreamDockXXX.h` 文件，检查你的设备的 **PID（Product ID）** 和 **VID（Vendor ID）** 是否包含在其中。
  如果不在列表中，请通过以下两种方式解决：
  1. 联系我们，我们可以协助你添加该设备；
  2. 或者你也可以根据示例，自行模仿类中的 `bool`类型 `registerX`变量注册即可

添加完成后重新编译即可识别。

### 5.3 代码编写注意事项

在设置设备图片时，SDK 通常为每种设备提供了两套接口函数：

- 一个是 **stream接口** 的版本（直接传入图像数据）
- 一个是 **file接口** 的版本（通过图像文件路径加载）

我们**强烈建议**优先使用 **file接口** 的版本。该方式更安全、更稳定，适用于大多数平台和设备。

#### ⚠️ 关于 `stream接口` 的接口函数

- 这些函数主要用于兼容一些第三方图像处理库（如你自己读取并处理图像内存数据的场景）；

#### ⚠️ 使用 `stream接口` 接口存在的风险

使用 **stream接口**需要极其小心，如果参数填写不当，可能对设备造成较大，严重的需要返厂重新烧录固件程序，例如：

- 图像数据越界导致设备内部卡死
- 图像大小/格式错误导致写入失败

因此，**请务必认真阅读 `streamdock.h` 中相关函数的注释说明**，确保理解每个参数的含义及使用方式。

错误使用此接口，后果自负。

### 5.4 新旧平台之间的差异

所谓新旧平台，其实指的是设备所使用的固件程序版本的不同。

你不需要了解过多技术细节，只需知道以下几点：

- 在本 SDK 中，**只有 293（包括 293s）是旧平台设备**；
- **N3、293V3、N4 均属于新平台设备**。

#### 🔁 操作行为差异

新旧平台在接口使用上的差异并不大，但它们在运行表现上的区别还是很明显的，主要体现在以下几点：

- **执行速度**：新平台设备的响应速度明显优于旧平台；
- **同步 / 异步执行**：
  - 新平台支持**设备异步执行命令**，即在读取返回信息的同时还能执行其他操作；
  - 旧平台（如 293）则不支持异步，所有命令必须**串行执行（同步）**；
- **等待与阻塞**：
  - 旧平台在很多操作中需要额外的阻塞等待；
  - 新平台只有在个别操作中需要等待，大多数操作可以连续快速执行。

#### 💡 ROM 与 RAM 操作差异

- `setBackgroundImgFile()` —— 设置开机背景图，写入的是设备的 **ROM**：
  - 写入速度较慢；
  - 但重启后仍然保留；
- `setKeyImgFile()` —— 设置按键图标，写入的是设备的 **RAM**：
  - 写入速度较快；
  - 但断电后会丢失，需重新设置。

> ⚠️ 因此在使用旧平台设备时，请格外注意命令执行的时机与节奏，避免因并发调用导致设备响应异常。

---

## 6. 🧪 使用示例

本节提供一个完整的 SDK 使用示例，包括以下几个方面：

- 枚举设备
- 唤醒屏幕，设置亮度
- 设置背景图片
- 设置按键图片

---

### 6.1 设置背景图片

```cpp
device->setEncoder(std::make_shared<OpenCVImageEncoder>());  // 在调用关联图片操作的接口时, 前提必须设置编码器
device->setBackgroundImgFile("YiFei.jpg");                   // 设置开机 logo 为路径 ./YiFei.jpg 的静态图片
device->refresh();
```

使用本地图片路径设置背景图。推荐使用该方式，简单安全。

### 6.2 设置按键图片

```cpp
device->setEncoder(std::make_shared<OpenCVImageEncoder>());  // 在调用关联图片操作的接口时, 前提必须设置编码器
device->setKeyImgFile("YiFei.jpg", 9);                       // 在按键 9 的位置设置路径为 ./YiFei.jpg 的静态图片
device->refresh();
```

为第 9 个按键位置设置图片，使用本地文件路径。

### 6.3 设置按键动图（必须是 `bool isDualDevice = true;`）

```cpp
device->setEncoder(std::make_shared<OpenCVImageEncoder>());  // 在调用关联图片操作的接口时, 前提必须设置编码器
device->gifer()->setKeyGifFile("1.gif", 1);                  // 在按键 1 的位置设置路径为 ./1.gif 的动态图片
device->gifer()->startGifLoop();                             // 启动 gif 发送线程
```

### 6.4 设置按键反馈回调处理

```cpp
device->reader()->startReadLoop();  // 调用注册按键读函数前或后, 必须开启读循环, 否则不会处理任何消息
// 此处第一个参数为注册的按键实际值, 需要查找这个值的你需要去`HotspotDevice/StreamDockXXX.cpp`里面查找`_readValueMap`中的`key`值
device->reader()->registerReadCallback(11, []()
		{ ToolKit::print("Key 11 pressed"); }, RegisterEvent::EveryThing);
```

### 6.5 主函数入口：设备枚举与调用测试函数

```cpp
ToolKit::disable_output = false;         // 禁用当前 sdk ToolKit打印
StreamDock::disableOutput(false);        // 启动 Transport 库内部打印
DeviceManager::instance().enumerator();  // 枚举当前可连接的 StreamDock 设备
// 监听设备热插拔，并设置其连接回调处理
DeviceManager::instance().listen([](std::shared_ptr<StreamDock> device) {
	doSomething(device);                   // 热插设备后, 会触发此处 dosomething 函数
	});
auto& streamdocks = DeviceManager::instance().getStreamDocks(); // 获取当前可连接的所有 StreamDock 设备列表
for (const auto& device : streamdocks) {
	try {
		doSomething(device.second);
	}
	catch (...) {
		std::cerr << "something error" << std::endl;
	}
}
```

🖼️ 示例图像文件请放置于 `bin/img` 目录下。
若出现找不到设备，请参考 [4.2 运行时找不到设备](#42-%E8%BF%90%E8%A1%8C%E6%97%B6%E6%89%BE%E4%B8%8D%E5%88%B0%E8%AE%BE%E5%A4%87-find-0-device)

#### 🔍 函数说明

- `open(AUTOREAD)`
  必须在使用设备前调用。
  `AUTOREAD` 启用自动监听事件，例如：

  - 按键按下
  - 图像更新成功
    要停止自动读取：

  ```cpp
  Transport::stopAutoRead();
  ```
- `init()`
  此函数作为一个一体化接口，用于自动唤醒、清除屏幕、设置亮度和刷新。
  通常在 `open()` 函数后使用。
- `asyncListen()`
  启动后台线程来监控设备插拔。
  它内部调用 `DeviceManager::listen()`。
- `joining_thread` 类
  这个类在当前作用域内自动 join 新创建的线程，使你无需手动管理线程的生命周期。
- `listen(bool autoReconnect = false)`
  如果 `autoReconnect = true`，它会在插入时自动调用 `open()` 和 `wakeScreen()`。
  你可以在以下位置添加自定义重连逻辑：

  ```cpp
  // 在这里重新连接并执行某些操作，比如启动信号调用函数
  ```

✅ 我们建议在启动时调用 `asyncListen()` 来监控实时设备变化。

---

## 7. 🆕 最新更新

### 版本 1.0.2 - 2025-11-18

#### 新增功能

- 新增设备支持：XL ,M3 ,N3系列, M18;

#### 依赖更新

- C++17 及以上标准
- [hidapi](https://github.com/libusb/hidapi) `v0.14.0`
- [libusb-1.0](https://libusb.info/) `v1.0.27`
- [libudev](https://github.com/systemd/systemd) `v245`（用于 Linux 设备监听）

---

## 8. 🔍 设备具体使用及快速上手

请参考 `src/test.h`文件，本SDK最简单的用法就是直接跟着该文件的对应的设备进行修改即可
