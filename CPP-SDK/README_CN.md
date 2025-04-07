# StreamDock C++ SDK 文档

## 1. 📘 本项目介绍

本项目是一个基于 C++17 开发的 SDK，旨在让用户能够通过代码直接控制 Stream Dock 设备。

该 SDK 提供了 Linux 与 Windows 的跨平台支持，用户可以根据自身的操作系统选择合适的构建方式进行编译和使用。目前 SDK 支持对部分设备的控制，例如：**293**、**293V3**、**N4**、**N3** 等型号，用户可通过查看第五节的使用示例，快速上手并调用我们提供的接口对设备进行操作。

SDK 中可能依赖一些第三方库，例如：

- **FreeImage**：用于图像加载与处理；
- **hidapi**：用于 USB 设备的数据传输与控制；
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
- **hidapi**（libusb 版本）：用于与 USB HID 设备通信

#### 安装命令（适用于 Ubuntu / Debian）

```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0
```

除此之外，SDK 还需要你安装 FreeImage 库。你可以访问以下仓库下载源码：👉 [https://github.com/xiaochufuji/FreeImage_clone](https://github.com/xiaochufuji/FreeImage_clone)) 进入 `FreeImage_clone` 文件夹后，执行以下命令进行编译和安装：

```bash
make -f Makefile.fip
sudo make -f Makefile.fip install
```

这些命令你都能在`Readme.linux`中看到

### 2.2 构建步骤

安装好依赖项后，进入项目主目录，你会看到如下结构：
```
├── bin
├── build
├── build.bat
├── build.sh
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
└── StreamDock_CppSDK
```
你可以直接在命令行中执行以下命令进行构建：

```bash
./build.sh
```

如果命令执行失败，请你使用`ll`命令查看`build.sh`的执行权限，给`build.sh`增加执行权限

```
chmod +x build.sh
```

当然你也可以使用自己的方式通过 CMake 构建，具体参数和配置可以查看 `build.sh` 脚本内容，自行构建。
⚠️ 构建期间如果出现找不到 `hid_get_input_report` 的错误，请参考第 [4.1](#41-%E7%BC%96%E8%AF%91%E6%97%B6%E6%8A%A5%E9%94%99undefined-reference-to-hid_get_input_report) 节进行处理。

### 2.3 运行示例

进入`bin`目录， 然后使用管理员权限执行即可，当然你可以修改cmake可执行文件输出的路径，但是这个时候你需要注意代码中自己修改的图片文件的相对路径

```
sudo ./main
```

---

## 3. 🪟 Windows 构建

### 3.1 环境准备

在 Windows 中，你并不需要安装太多的第三方库，因为我们已经提前将所需的库编译成了动态链接库，并放在了 `bin` 目录中。你只需要关注自己的环境配置即可。

我们的测试环境如下：

- 编译器：Visual Studio 17 2022
- 语言标准：C++17（也支持更新版本）

如果你也是使用 Visual Studio 2022 且开启了 C++17 或更新的标准，那么你可以直接跳转到构建章节查看构建方法。

如果你的环境和我们的不同，请继续阅读本节内容了解构建步骤。

目前本 SDK 只使用 **MSVC 工具链** 进行了完整测试，因此我们推荐使用 **MSVC** 进行构建。

在使用 CMake 构建时，可以通过 `-G` 参数显式指定构建器，例如：

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
```

### 💡 关于 C++ 标准支持

SDK 默认使用 **C++17** 进行开发和构建，原因是代码中使用了部分 C++17 的新特性（如 `std::filesystem`）。

如果你希望使用 **C++14** 或 **C++11** 进行构建，可以自行修改代码。修改量并不大，主要集中在 `std::filesystem` 的替换部分，改为传统文件路径操作即可。
⚠️ 如果你希望使用 MinGW 或其他编译器构建本项目，请自行测试兼容性，我们未进行适配。

### 3.2 使用 CMake 构建

你可以将整个工程文件夹（即 `bin` 目录的上一层目录）直接拖入 Visual Studio 2022 中。

此时 Visual Studio 会自动识别并配置 CMake 项目，你只需等待其完成加载和配置。

配置完成后，按下以下任一快捷键进行构建：

- `Ctrl + B`
- `Ctrl + Shift + B`

构建成功后，生成的可执行文件将位于 `bin` 目录下。

如果你不使用 Visual Studio 进行代码编辑或者更喜欢命令行方式，也可以使用 **Visual Studio 提供的命令行工具**：

> `x64 Native Tools Command Prompt for VS 2022`

进入项目文件夹后，执行以下命令：

```bat
build.bat
```

这样也可以完成构建。

### 3.3 注意事项

我们已经提前编译好了所有必要的第三方库，并在 SDK 中提供了完整的动态链接库，包括但不限于：

- `FreeImage.dll`
- `FreeImagePlus.dll`
- `hidapi.dll`

这些动态库都在bin文件夹中，你只需要在编译 SDK 后，将这些 `.dll` 文件与编译后生成的可执行程序放在同一目录下（例如 `bin/` 目录），即可正常运行程序，无需单独安装这些库。

---

## 4. ⚠️ 注意事项

### 4.1 编译时报错：undefined reference to `hid_get_input_report`

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

假设说找到的路径为`/usr/lib/x86_64-linux-gnu/libhidapi-libusb.so.0`，那么你只需要使用如下命令进行替换即可:

```
sudo cp ./lib/libhidapi-libusb.so.0 /usr/lib/x86_64-linux-gnu
```

### 4.2 运行时找不到设备 `find [0] device`

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
  请打开项目中的 `ProductIDs.h` 文件，检查你的设备的 **PID（Product ID）** 和 **VID（Vendor ID）** 是否包含在其中。
  如果不在列表中，请通过以下两种方式解决：
  1. 联系我们，我们可以协助你添加该设备；
  2. 或者你也可以根据示例，自行在 `ProductIDs.h` 中添加你的设备信息。

添加完成后重新编译即可识别。

### 4.3 代码编写注意事项

在设置设备图片时，SDK 通常为每种设备提供了两套接口函数：

- 一个是 **带 `data` 参数** 的版本（直接传入图像数据）
- 一个是 **不带 `data` 参数** 的版本（通过图像文件路径加载）

我们**强烈建议**优先使用 **不带 `data` 参数** 的版本。该方式更安全、更稳定，适用于大多数平台和设备。

#### ⚠️ 关于带 `data` 的接口函数

- 这些函数主要用于兼容一些第三方图像处理库（如你自己读取并处理图像内存数据的场景）；
- 除了旧型号设备（如 293）外，其他设备使用 `data` 接口的意义并不大；
- 后续版本我们可能会增加一个新接口，支持直接内存流发送（`memory data`），提升图像传输速度；
- **当前阶段仍优先推荐使用文件路径接口**。

#### ⚠️ 使用 `data` 接口存在的风险

使用带 `data` 参数的接口需要极其小心，如果参数填写不当，可能对设备造成较大，严重的需要返厂重新烧录固件程序，例如：

- 图像数据越界导致设备内部卡死
- 图像大小/格式错误导致写入失败

因此，**请务必认真阅读 `StreamDock.h` 中相关函数的注释说明**，确保理解每个参数的含义及使用方式。

错误使用此接口，后果自负。

### 4.4 新旧平台之间的差异

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

- `setBackground()` —— 设置开机背景图，写入的是设备的 **ROM**：
  - 写入速度较慢；
  - 但重启后仍然保留；
- `setKeyImg()` —— 设置按键图标，写入的是设备的 **RAM**：
  - 写入速度较快；
  - 但断电后会丢失，需重新设置。

> ⚠️ 因此在使用旧平台设备时，请格外注意命令执行的时机与节奏，避免因并发调用导致设备响应异常。

---

## 5. 🧪 使用示例

本节提供一个完整的 SDK 使用示例，包括以下几个方面：

- 枚举设备并打开
- 唤醒屏幕，设置亮度
- 设置背景图片（文件 / 内存数据）
- 设置按键图片（文件 / 内存数据）

---

### 5.1 设置背景图片（使用文件路径）

```cpp
void test1(const std::shared_ptr<StreamDock> &s, bool test)
{
    if (!test)
        return;
    s->setBackgroundImg("./img/YiFei320.png");
    s->refresh();
}
```

使用本地图片路径设置背景图。推荐使用该方式，简单安全。

### 5.2 设置背景图片（使用内存数据）

```cpp
void test2(const std::shared_ptr<StreamDock> &s, bool test)
{
    if (!test)
        return;
    fipImage img;
    img.load("./img/YiFei.png");
    s->setBackgroundImgData(img.accessPixels(), RgbaFormat::RGBA, 800, 480);
    s->refresh();
}
```

使用内存中加载的像素数据设置背景图。需要确保图像格式和尺寸匹配。
⚠️ 图像格式和尺寸都为当前在内存中图像数据的格式和尺寸。

### 5.3 设置按键图片（使用文件路径）

```cpp
void test3(const std::shared_ptr<StreamDock> &s, bool test)
{
    if (!test)
        return;
    s->setKeyImg("./img/tiga112.png", 3);
    s->refresh();
}
```

为第 3 个按键设置图片，使用本地文件路径。

### 5.4 设置按键图片（使用内存数据）

```cpp
void test4(const std::shared_ptr<StreamDock> &s, bool test)
{
    if (!test)
        return;
    fipImage img;
    img.load("./img/tiga112.png");
    s->setKeyImgData(img.accessPixels(), 2, RgbaFormat::RGBA, 112, 112);
    s->refresh();
}
```

为第 2 个按键设置图像像素数据。使用内存数据版本时，请特别注意格式正确性。
⚠️ 图像格式和尺寸都为当前在内存中图像数据的格式和尺寸。

### 5.5 主函数入口：设备枚举与调用测试函数

```cpp
int main()
{
    std::unique_ptr<DeviceManager> manager(new DeviceManager());
	  auto streamDocks = manager->enumerate();
	  std::cout << "find [" << streamDocks.size() << "] device" << "\n";
	  manager->asyncListen();
	  joining_thread j;
	  for (auto it = streamDocks.begin(); it != streamDocks.end(); it++)
	  {
	  	// get device manager instance
	  	auto s = it->second;
	  	s->open();
	  	s->init();
	  	j = joining_thread(std::thread([](decltype(s) st) {
	  		while (1)
	  		{
	  			auto a = st->read();
	  			auto str = StreamDock::parseRead(a);
	  			if (!str.empty()) std::cout << str << std::endl;
	  		}
	  		}, s));
	  	test1(s, true);
	  	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	  	test2(s, true);
	  	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	  	test3(s, true);
	  	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	  	test4(s, true);
	  }
}
```

🖼️ 示例图像文件请放置于 `bin/img/` 目录下。
若出现找不到设备，请参考 [4.2 运行时找不到设备](#42-%E8%BF%90%E8%A1%8C%E6%97%B6%E6%89%BE%E4%B8%8D%E5%88%B0%E8%AE%BE%E5%A4%87-find-0-device)

#### 🔍 函数说明

* `open(AUTOREAD)`
  在操作设备之前，必须先调用 `open()` 进行连接。
  其中参数 `AUTOREAD` 的作用是自动开启设备的返回消息监听，例如：
  
  * 按键按下事件
  * 设置背景图成功通知
    如果你希望关闭自动读取功能，可以调用：
    ```cpp
    Transport::stopAutoRead();
    ```
* `init()`
  该函数是自动唤醒、清屏、设置亮度、刷新一体的函数接口
  通常可以用于`open()`函数之后
* `asyncListen()`
  该函数会启动异步设备监听，用于检测设备的插拔操作。
  实际上是启动了一个线程调用了 `DeviceManager::listen()` 函数。
* `DeviceManager::listen()` 支持一个布尔参数：
    ```cpp
    int listen(bool autoReconnect = false);
    ```
* `joining_thread`类
  这个类自动帮助你在当前作用域将新开辟的线程join而无需太多关注线程的生命周期
* 当 `autoReconnect = true` 时，监听到插入新设备后会自动执行 `open()` 和 `wakeScreen()`，在`open()`和`wakeup()`之后还有一行注释，你可以在此处增加你自己的重连后的逻辑；
    ```cpp
    // reconnect and do something here, like launch a singal to call a function
    ```
* 否则你需要手动处理设备插入逻辑。

✅ 建议在主程序启动后立即调用 `asyncListen()`，以便动态响应设备插拔行为。

---
