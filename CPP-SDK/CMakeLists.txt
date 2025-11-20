cmake_minimum_required(VERSION 3.16)
project(HIDProject)

# 为 MSVC 设置运行时库 - 必须在 project() 之后立即设置
if(MSVC)
    # 启用新的 MSVC 运行时库策略
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    # 强制移除默认的 /MDd 和 /MTd 标志，确保使用正确的运行时库
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/MDd" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MTd" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MD" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MT" "" ${flag_var} "${${flag_var}}")
    endforeach()

    # 为不同配置显式设置运行时库
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MD")
endif()

set(TARGETNAME "main")

# 添加构建选项
option(BUILD_LIBHIDCPP_SHARED "Build shared libhidcpp libraries" ON)
option(USE_SHARED_HIDAPI "Link with shared hidapi" ON)

# 可执行程序生成路径 - 分离 Debug 和 Release
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/bin/RelWithDebInfo)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_SOURCE_DIR}/bin/MinSizeRel)

# 库文件输出路径 - 分离 Debug 和 Release
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/Release)

# 引入 hidapi
include(cmake/FetchHid.cmake)
add_subdirectory(ImgProcesser)
add_subdirectory(src/Transport)

# 收集 Hotspot 设备信息
file(GLOB_RECURSE HOTSPOTDEVICE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/HotspotDevice/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/HotspotDevice/**/*.h"
)

# 可执行程序
add_executable(${TARGETNAME}
    src/main.cpp
    ${HOTSPOTDEVICE}
    "src/DeviceInfo/streamdockinfo.h"
    "src/DeviceInfo/streamdock.h"
    "src/DeviceInfo/streamdock.cpp"
    "src/DeviceInfo/streamdockfactory.h"
    "src/DeviceInfo/streamdockfactory.cpp"
    "src/DeviceInfo/featureoption.h"
    "src/DeviceInfo/Feature/RGBController/rgbcontroller.cpp"
    "src/DeviceInfo/Feature/GifController/gifcontroller.cpp"
    "src/DeviceInfo/Feature/ReadController/readcontroller.cpp"
    "src/DeviceInfo/Feature/Configer/configer.cpp"
    "src/DeviceInfo/Feature/HeartBeat/heartbeat.cpp"
    "src/DeviceManager/DeviceEnumerator/deviceenumerator.h"
    "src/DeviceManager/DeviceEnumerator/deviceenumerator.cpp"
    "src/DeviceManager/devicemanager.cpp" "src/DeviceManager/devicemanager_win.cpp"
    "src/DeviceManager/devicemanager.cpp" "src/DeviceManager/devicemanager_linux.cpp"
    "src/DeviceManager/devicemanager.cpp" "src/DeviceManager/devicemanager_mac.cpp"
    "src/ToolKit/toolkit.h"
)

# 链接 
target_link_libraries(${TARGETNAME} PRIVATE TransportCWrapper ImgProcesser) # 依赖库
target_include_directories(${TARGETNAME} # 依赖头文件
    PRIVATE
    "src"
    "src/DeviceEnumerator"
    "src/DeviceInfo"
    "src/Transport"
    "src/ToolKit"
)
target_compile_features(${TARGETNAME} PRIVATE cxx_std_17)

# Windows 下确保 Transport DLL 被复制
if(WIN32)
    # 添加对 Transport DLL 复制目标的依赖
    if(TARGET copy_transport_dll_debug)
        add_dependencies(${TARGETNAME} copy_transport_dll_debug)
    endif()
    if(TARGET copy_transport_dll_release)
        add_dependencies(${TARGETNAME} copy_transport_dll_release)
    endif()
endif()

# 额外链接
if(WIN32)
    target_link_libraries(${TARGETNAME} PRIVATE setupapi) # windows 额外需要使用setupapi
elseif(APPLE)
    find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(APPLICATIONSERVICES_FRAMEWORK ApplicationServices)

    target_link_libraries(${TARGETNAME}
        PRIVATE
        ${COREFOUNDATION_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${APPLICATIONSERVICES_FRAMEWORK}
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${TARGETNAME}
        PRIVATE
        udev
        pthread
    )
endif()

# Windows下复制需要的DLL到可执行文件目录
if(WIN32)
    set(OpenCV_BIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ImgProcesser/third_party/opencv/windows/x64/vc17/bin")

    # 定义需要复制的 OpenCV DLL 文件（不带后缀）
    set(OpenCV_DLL_NAMES
        "opencv_core4120"
        "opencv_imgcodecs4120"
        "opencv_imgproc4120"
    )

    # 为每个 OpenCV DLL 创建复制命令
    foreach(dll_name ${OpenCV_DLL_NAMES})
        add_custom_command(TARGET ${TARGETNAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OpenCV_BIN_PATH}/${dll_name}$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${TARGETNAME}>/${dll_name}$<$<CONFIG:Debug>:d>.dll"
            COMMENT "📦 Copying ${dll_name}$<$<CONFIG:Debug>:d>.dll to output directory"
        )
    endforeach()
endif()
