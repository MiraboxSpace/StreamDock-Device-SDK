# StreamDock C++ SDK Documentation

## 1. 📘 Project Overview

This project is a C++17-based SDK that allows users to directly control Stream Dock devices through code.

The SDK supports both Linux and Windows platforms. Users can choose the appropriate build method based on their system. Currently, the SDK supports controlling several device models such as: **293**, **293V3**, **N4**, **N3**, **N1**, etc. You can refer to Section 5 for usage examples to quickly get started with the provided interfaces.

The SDK may rely on some third-party libraries, including:

- **OpenCV**: for image loading and processing;
- **hidapi**: for USB device data transmission and control;
- **giflib**: for processing animated gif images;
- Other system-specific dependencies (e.g., libudev).

Please refer to Section 2 (Linux Build) or Section 3 (Windows Build) for setup instructions based on your platform.

---

## 2. 🐧 Linux Build

### 2.1 Dependencies Installation

Before building the SDK, make sure the following dependencies are installed:

#### Required Packages

- **CMake**: build system
- **libudev-dev**: to detect USB device events (Linux only)
- **libusb-1.0-0-dev**: dependency for hidapi (libusb version)
- **libgif-dev**: for gif image processing

#### Installation Command (for Ubuntu / Debian)

```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install -y libudev-dev libusb-1.0-0-dev=2:1.0.23-2build1 libgif-dev
```

Additionally, the SDK requires the OpenCV library.

```bash
sudo apt install libopencv-dev
```

You can also use a trimmed `OpenCV` library. You can trim it using `CMake-GUI` or command line. The goal is to read most image types and support encoding them to `BGR888`, `BGR565`, `jpeg`, and `png` types to ensure the `SDK` runs properly. If you find trimming troublesome, please install directly using the command above.

### 2.2 Build Steps

After installing dependencies, go to the project root directory. You will see the following structure:

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

Run the following command to build:

```bash
chmod +x build.sh
./build.sh
```

You can also build manually using CMake. Refer to `build.sh` for configuration details.

⚠️ If an error like `hid_get_input_report` not found occurs during linking, refer to Section [4.1](#41-compile-errorundefined-reference-to-hid_get_input_report) for resolution.

### 2.3 Run Example

Navigate to the `bin` directory and execute the binary with root permission:

```bash
sudo ./main
```

If you don't want to use sudo, you need to add your device to your user's device permission list. Of course, you can modify the cmake executable file output path, but at this time you need to pay attention to the relative paths of image files modified in the code.

---

## 3. 🪟 Windows Build

### 3.1 Environment Setup

On Windows, you don't need to manually install most third-party libraries. We've already precompiled the required `.dll` files and included them in the `bin` directory.

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

We've precompiled all required third-party libraries and included them in the SDK's `bin` directory:

- `opencv_core4120.dll`
- `opencv_imgcodecs4120.dll`
- `opencv_imgproc4120.dll`
- `hidapi.dll`

After building the SDK, simply copy these `.dll` files into the same directory as the executable (e.g., `bin/`), and you're ready to run — no extra installation needed.

---

## 4. 🍎 macOS Build

### 4.1 Dependencies Installation

On macOS, the SDK includes pre-built OpenCV libraries for macOS. Before building, you need to prepare the OpenCV dependencies.

Navigate to the `ImgProcesser/third_party` directory and run the following script to create symbolic links for OpenCV libraries:

```bash
bash link_opencv_symlinks.sh
```

This script will create the necessary symlinks in `opencv/mac/lib` to allow CMake to properly locate the OpenCV libraries.

### 4.2 Build Steps

After preparing the dependencies, go to the project root directory. You will see the following structure:

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

Run the following command to build:

```bash
chmod +x build.sh
./build.sh
```

You can also build manually using CMake. Refer to `build.sh` for configuration details.

## 5. ⚠️ Troubleshooting

### 4.1 Compile Error: undefined reference to `hid_get_input_report`

On Linux, you may encounter the following error if the system-installed `hidapi` version is incompatible:

```text
/usr/bin/ld: /home/xxx/StreamDock_CppSDK/StreamDock_CppSDK/lib/libtransport.a: undefined reference to `hid_get_input_report'
collect2: error: ld returned 1 exit status
make[2]: *** [StreamDock_CppSDK/CMakeFiles/main.dir/build.make:197: /home/xxx/StreamDock_CppSDK/bin/main] Error 1
make[1]: *** [CMakeFiles/Makefile2:128: StreamDock_CppSDK/CMakeFiles/main.dir/all] Error 2
make: *** [Makefile:91: all] Error 2
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

  Lack of administrator privileges will prevent the device from being accessed correctly.
- **Device PID/VID not registered**
  Check the `StreamDockXXX.h` file to confirm your device's PID/VID is listed. If not:

  1. Contact us to add your device;
  2. Or manually add your device entry based on existing examples by imitating the `bool` type `registerX` variable in the class.

After adding, recompile to recognize the device.

### 4.3 Code Writing Guidelines

When setting device images, the SDK usually provides two sets of interface functions for each device:

- A **stream interface** version (directly passing image data)
- A **file interface** version (loading through image file path)

We **strongly recommend** prioritizing the **file interface** version. This method is safer and more stable, suitable for most platforms and devices.

#### ⚠️ About the `stream interface` functions

- These functions are mainly used for compatibility with some third-party image processing libraries (such as scenarios where you read and process image memory data yourself);

#### ⚠️ Risks of using `stream interface`

Using the **stream interface** requires extreme care. If parameters are filled incorrectly, it may cause significant damage to the device, and in severe cases, the device may need to be returned to the factory for firmware reprogramming, for example:

- Image data out of bounds causing device internal deadlock
- Image size/format errors causing write failures

Therefore, **please carefully read the comments of related functions in `streamdock.h`** to ensure you understand the meaning and usage of each parameter.

Incorrect use of this interface is at your own risk.

### 4.4 Differences Between New and Old Platforms

The term "platform" refers to the device's firmware generation.

You don't need to understand too many technical details, just know the following:

- In this SDK, **only 293 (including 293s) are old platform devices**;
- **N3, 293V3, N4 all belong to new platform devices**.

#### 🔁 Behavioral Differences

The differences between new and old platforms in interface usage are not significant, but their differences in runtime performance are quite obvious, mainly reflected in the following points:

- **Execution speed**: New platform devices respond significantly faster than old platforms;
- **Synchronous / Asynchronous execution**:
  - New platforms support **asynchronous command execution**, meaning they can perform other operations while reading return information;
  - Old platforms (like 293) do not support asynchronous, all commands must be **executed serially (synchronously)**;
- **Waiting and blocking**:
  - Old platforms require additional blocking waits in many operations;
  - New platforms only need to wait in individual operations, most operations can be executed continuously and quickly.

#### 💡 ROM vs RAM Operation Differences

- `setBackgroundImgFile()` - Set startup background image, writes to the device's **ROM**:
  - Slower write speed;
  - But remains after restart;
- `setKeyImgFile()` - Set key icon, writes to the device's **RAM**:
  - Faster write speed;
  - But is lost after power off, needs to be reset.

> ⚠️ Therefore, when using old platform devices, please pay special attention to the timing and rhythm of command execution to avoid device response abnormalities caused by concurrent calls.

---

## 5. 🧪 Usage Example

This section provides a complete SDK usage example, including the following aspects:

- Enumerating devices
- Waking screen, setting brightness
- Setting background image
- Setting key image

---

### 5.1 Set Background Image

```cpp
device->setEncoder(std::make_shared<OpenCVImageEncoder>());  // When calling interfaces related to image operations, the premise must be to set the encoder
device->setBackgroundImgFile("YiFei.jpg");                   // Set startup logo to a static image at path ./YiFei.jpg
device->refresh();
```

Use local image path to set background image. This method is recommended for simplicity and safety.

### 5.2 Set Key Image

```cpp
device->setEncoder(std::make_shared<OpenCVImageEncoder>());  // When calling interfaces related to image operations, the premise must be to set the encoder
device->setKeyImgFile("YiFei.jpg", 9);                       // Set a static image at path ./YiFei.jpg at key position 9
device->refresh();
```

Set an image for the 9th key position, using local file path.

### 5.3 Set Key Animated Image (must be `bool isDualDevice = true;`)

```cpp
device->setEncoder(std::make_shared<OpenCVImageEncoder>());  // When calling interfaces related to image operations, the premise must be to set the encoder
device->gifer()->setKeyGifFile("1.gif", 1);                  // Set an animated image at path ./1.gif at key position 1
device->gifer()->startGifLoop();                             // Start gif sending thread
```

### 5.4 Set Key Feedback Callback Handling

```cpp
device->reader()->startReadLoop();  // Before or after calling the register key read function, you must start the read loop, otherwise no messages will be processed
// The first parameter here is the actual value of the registered key. To find this value, you need to go to `HotspotDevice/StreamDockXXX.cpp` to find the `key` value in `_readValueMap`
device->reader()->registerReadCallback(11, []()
		{ ToolKit::print("Key 11 pressed"); }, RegisterEvent::EveryThing);
```

### 5.5 Main Function Entry: Device Enumeration and Calling Test Functions

```cpp
ToolKit::disable_output = false;         // Disable current sdk ToolKit printing
StreamDock::disableOutput(false);        // Enable Transport library internal printing
DeviceManager::instance().enumerator();  // Enumerate currently connectable StreamDock devices
// Listen for device hot-plug and set its connection callback handling
DeviceManager::instance().listen([](std::shared_ptr<StreamDock> device) {
	doSomething(device);                   // After hot-plugging the device, the dosomething function here will be triggered
	});
auto& streamdocks = DeviceManager::instance().getStreamDocks(); // Get a list of all currently connectable StreamDock devices
for (const auto& device : streamdocks) {
	try {
		doSomething(device.second);
	}
	catch (...) {
		std::cerr << "something error" << std::endl;
	}
}
```

🖼️ Example image files should be placed in the `bin/img` directory.
If no devices are found, please refer to [4.2 Device Not Found](#42-device-not-found-find-0-device)

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
  This function serves as an all-in-one interface for automatic wake-up, screen clearing, brightness setting, and refreshing.
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

---

## 6. 🆕 Latest Updates

### Version 1.0.2 - 2025-11-18

#### New Features

- Added device support: XL, M3, N3 series, M18;

#### Dependency Updates

- C++17 and above standards
- [hidapi](https://github.com/libusb/hidapi) `v0.14.0`
- [libusb-1.0](https://libusb.info/) `v1.0.27`
- [libudev](https://github.com/systemd/systemd) `v245` (for Linux device listening)

---

## 7. 🔍 Device Specific Usage and Quick Start

Please refer to the `src/test.h` file. The simplest way to use this SDK is to directly modify it according to the corresponding device in this file.
