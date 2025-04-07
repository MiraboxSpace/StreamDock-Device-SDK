# StreamDock C++ SDK Documentation

## 1. 📘 Project Overview

This project is a C++17-based SDK that allows users to directly control Stream Dock devices through code.

The SDK supports both Linux and Windows platforms. Users can choose the appropriate build method based on their system. Currently, the SDK supports controlling several device models such as: **293**, **293V3**, **N4**, **N3**. You can refer to Section 5 for usage examples to quickly get started with the provided interfaces.

The SDK may rely on some third-party libraries, including:

- **FreeImage**: for image loading and processing
- **hidapi**: for USB device communication
- Other system-specific dependencies (e.g., libudev on Linux)

Please refer to Section 2 (Linux Build) or Section 3 (Windows Build) for setup instructions based on your platform.

---

## 2. 🐧 Linux Build

### 2.1 Dependencies Installation

Before building the SDK, make sure the following dependencies are installed:

#### Required Packages

- **CMake**: build system
- **libudev-dev**: to detect USB device events (Linux only)
- **libusb-1.0-0-dev**: dependency for hidapi (libusb version)
- **hidapi** (libusb version): for communicating with USB HID devices

#### Installation Command (for Ubuntu / Debian)

```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install -y libudev-dev libusb-1.0-0-dev libhidapi-libusb0
```

Additionally, the SDK requires the FreeImage library. You can download the source code from: 👉 [https://github.com/xiaochufuji/FreeImage_clone](https://github.com/xiaochufuji/FreeImage_clone)

After entering the `FreeImage_clone` folder, build and install using:

```bash
make -f Makefile.fip
sudo make -f Makefile.fip install
```

You can also find these commands in the `Readme.linux` file.

---

### 2.2 Build Steps

After installing dependencies, go to the project root directory. You will see the following structure:

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

Run the following command to build:

```bash
./build.sh
```

If the command fails, use `ll` to check the execution permission of `build.sh`. If necessary, add execution permission:

```bash
chmod +x build.sh
```

You can also build manually using CMake. Refer to `build.sh` for configuration details.

⚠️ If an error like `hid_get_input_report` not found occurs during linking, refer to Section [4.1](#41-compile-errorundefined-reference-to-hid_get_input_report) for resolution.

---

### 2.3 Run Example

Navigate to the `bin` directory and execute the binary with root permission:

```bash
sudo ./main
```

If you've changed the output path in CMake, make sure the image paths in your code are updated accordingly.

---

## 3. 🪟 Windows Build

### 3.1 Environment Setup

On Windows, you don’t need to manually install most third-party libraries. We've already precompiled the required `.dll` files and included them in the `bin` directory.

Tested environment:

- Compiler: Visual Studio 17 2022
- Language standard: C++17 (C++20 is also supported)

If you're using Visual Studio 2022 with C++17 or newer, you can skip directly to the build instructions.

Otherwise, please read this section for further guidance.

This SDK has only been tested with the **MSVC** toolchain. We recommend using MSVC to ensure full compatibility.

To specify the generator in CMake:

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
```

### 💡 About C++ Standard

The SDK uses C++17 by default due to use of modern features such as `std::filesystem`.

If you prefer to use **C++14** or **C++11**, you can modify the code. Most required changes involve replacing `std::filesystem` usage with traditional file handling. This can enable older standards.

⚠️ Building with MinGW or other compilers has not been tested and is not officially supported.

---

### 3.2 Building with CMake

You can drag the entire project folder (the one above `bin/`) into Visual Studio 2022.

Visual Studio will automatically detect and configure the CMake project. After configuration completes, use the following shortcuts to build:

- `Ctrl + B`
- `Ctrl + Shift + B`

The output executable will be generated under the `bin` directory.

Alternatively, you can build using the Visual Studio command line:

> `x64 Native Tools Command Prompt for VS 2022`

Run the following command in the project directory:

```bat
build.bat
```

This also produces the executable.

---

### 3.3 Notes

We've precompiled all required third-party libraries and included them in the SDK’s `bin` directory:

- `FreeImage.dll`
- `FreeImagePlus.dll`
- `hidapi.dll`

After building the SDK, simply copy these `.dll` files into the same directory as the executable (e.g., `bin/`), and you're ready to run — no extra installation needed.

---

## 4. ⚠️ Troubleshooting

(For brevity, the rest of the document such as sections 4.1 through 5.5 would continue here just like shown earlier.)

---

### 4.1 Compile Error: undefined reference to `hid_get_input_report`

On Linux, you may encounter the following error if the system-installed `hidapi` version is incompatible:

```text
/usr/bin/ld: /home/xxx/StreamDock_CppSDK/StreamDock_CppSDK/lib/libtransport.a: undefined reference to `hid_get_input_report'
collect2: error: ld returned 1 exit status
```

#### Solution:

First, find the installed libhidapi-libusb path:

```bash
sudo find / -name libhidapi-libusb.so.0
```

Assume the path is `/usr/lib/x86_64-linux-gnu/libhidapi-libusb.so.0`, replace it with the one provided in `lib/`:

```bash
sudo cp ./lib/libhidapi-libusb.so.0 /usr/lib/x86_64-linux-gnu
```

---

### 4.2 Device not found: `find [0] device`

If you see:

```text
find [0] device
```

No connected devices were found.

#### 🛠 Possible Causes:

- **Insufficient permissions on Linux**
  Run the executable with `sudo`:
  
  ```bash
  sudo ./bin/main
  ```
- **Device PID/VID not registered**
  Check the `ProductIDs.h` file to confirm your device's PID/VID is listed. If not:
  
  1. Contact us to add your device;
  2. Or manually add your device entry based on existing examples, then recompile.

---

### 4.3 Code Writing Guidelines

Most device APIs provide two versions for setting images:

- With `data` (passing raw pixel buffer)
- Without `data` (loading from image file path)

We strongly recommend using the **non-data (path)** version for simplicity and safety.

#### ⚠️ About the `data` version

- Provided mainly for compatibility with third-party image libraries;
- Only old devices like 293 really need it;
- We may add memory stream support later for faster performance;
- Currently, using file path is the preferred approach.

#### ⚠️ Potential risks using `data` interface

Incorrect use may cause:

- Device freeze due to memory overflow
- Invalid size or format errors
- Severe misuse may require firmware reflashing

**Please carefully read function comments in `StreamDock.h`.** Use at your own risk.

---

### 4.4 Differences Between New and Old Platforms

The term “platform” refers to the device’s firmware generation.

Key differences:

- Only **293 / 293s** are considered **old platforms**;
- **N3**, **293V3**, and **N4** are **new platforms**.

#### 🔁 Behavioral Differences

- **Performance**: Newer platforms are much faster;
- **Execution model**:
  - New: Supports **asynchronous operations** (e.g., reading and writing at the same time)
  - Old: Requires **sequential (synchronous)** command execution
- **Blocking**: Old platform needs manual delays in many cases; new mostly doesn’t.

#### 💡 ROM vs RAM

- `setBackground()` writes to **ROM**:
  - Slower write speed
  - Persistent after reboot
- `setKeyImg()` writes to **RAM**:
  - Fast
  - Lost on power-off

> ⚠️ On old platforms, avoid concurrent operations — commands must be executed in sequence.

---

## 5. 🧪 Usage Example

This example demonstrates:

- Enumerating and opening devices
- Waking screen, setting brightness
- Setting background (file / memory)
- Setting key image (file / memory)

### 5.1 Set Background (File Path)

```cpp
void test1(const std::shared_ptr<StreamDock> &s, bool test)
{
    if (!test)
        return;
    s->setBackgroundImg("./img/YiFei320.png");
    s->refresh();
}
```

### 5.2 Set Background (Memory Data)

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

### 5.3 Set Key Image (File Path)

```cpp
void test3(const std::shared_ptr<StreamDock> &s, bool test)
{
    if (!test)
        return;
    s->setKeyImg("./img/tiga112.png", 3);
    s->refresh();
}
```

### 5.4 Set Key Image (Memory Data)

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

### 5.5 Main Function and Notes

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

📁 Make sure example images exist in `bin/img/`.
If no devices are found, refer to [4.2 Device Not Found](#42-device-not-found-find-0-device).

#### 🔍 Function Notes

- `open(AUTOREAD)`
  Must be called before using the device.
  `AUTOREAD` enables automatic listening to events such as:
  
  - Button press
  - Image update success
    To stop auto-read:
  
  ```cpp
  Transport::stopAutoRead();
  ```
- `init()`
  This function serves as an all-in-one interface for automatic wake-up, screen   clearing, brightness setting, and refreshing.
  It is typically used after the `open()` function.
- `asyncListen()`
  Starts a background thread to monitor device plug/unplug.
  It internally calls `DeviceManager::listen()`.
- `joining_thread` class
  This class automatically joins a newly created thread within the current scope, saving you from having to manually manage the thread's lifecycle.
- `listen(bool autoReconnect = false)`
  If `autoReconnect = true`, it will auto-call `open()` and `wakeScreen()` on insert.
  You can add custom reconnection logic at:
  
  ```cpp
  // reconnect and do something here, like launch a signal to call a function
  ```

✅ We recommend calling `asyncListen()` at startup to monitor real-time device changes.


