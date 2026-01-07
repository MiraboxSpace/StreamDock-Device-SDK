cmake_minimum_required(VERSION 3.16)
project(HIDProject)

# Set the MSVC runtime library - must be set immediately after project()
if(MSVC)
    # Enable the new MSVC runtime library policy
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    # Force-remove the default /MDd and /MTd flags to ensure the correct runtime library is used
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/MDd" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MTd" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MD" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MT" "" ${flag_var} "${${flag_var}}")
    endforeach()

    # Explicitly set the runtime library for each configuration
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MD")
endif()

set(TARGETNAME "main")

# Add build options
option(BUILD_LIBHIDCPP_SHARED "Build shared libhidcpp libraries" ON)
option(USE_SHARED_HIDAPI "Link with shared hidapi" ON)

# Check for source files; use prebuilt libraries if missing
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/transport_c.cpp")
    set(TRANSPORTDLL_USE_IMPORTED ON CACHE BOOL "Use prebuilt transport library instead of building from source" FORCE)
    message(STATUS "Source files not found, using prebuilt transport library")
    
    # Check whether the prebuilt library exists
    if(WIN32)
        if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/transport.dll" AND
           NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/transportd.dll")
            message(FATAL_ERROR "Prebuilt transport DLL not found in ${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/")
        endif()
    elseif(APPLE)
        # Check macOS architecture
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
            # ARM macOS
            if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/libtransport_arm64.dylib")
                message(FATAL_ERROR "Prebuilt transport library not found: ${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/libtransport_arm64.dylib")
            endif()
        else()
            # Intel macOS
            if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/libtransport.dylib")
                message(FATAL_ERROR "Prebuilt transport library not found: ${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/libtransport.dylib")
            endif()
        endif()
    elseif(UNIX AND NOT APPLE)
        # Check Linux architecture
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
            # ARM Linux
            if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/libtransport_arm64.so")
                message(FATAL_ERROR "Prebuilt transport library not found: ${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/libtransport_arm64.so")
            endif()
        else()
            # x86_64 Linux
            if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/libtransport.so")
                message(FATAL_ERROR "Prebuilt transport library not found: ${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/TransportDLL/libtransport.so")
            endif()
        endif()
    endif()
endif()

# Executable output paths - separate Debug and Release
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/bin/RelWithDebInfo)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_SOURCE_DIR}/bin/MinSizeRel)

# Library output paths - separate Debug and Release
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/Release)

# Include hidapi
include(cmake/FetchHid.cmake)
add_subdirectory(ImgProcesser)
add_subdirectory(src/Transport)

# Collect Hotspot device sources
file(GLOB_RECURSE HOTSPOTDEVICE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/HotspotDevice/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/HotspotDevice/**/*.h"
)

# Executable
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

# Link
target_link_libraries(${TARGETNAME} PRIVATE TransportCWrapper ImgProcesser TransportDLL) # Dependencies
target_include_directories(${TARGETNAME} # Dependency headers
    PRIVATE
    "src"
    "src/DeviceEnumerator"
    "src/DeviceInfo"
    "src/Transport"
    "src/ToolKit"
)
target_compile_features(${TARGETNAME} PRIVATE cxx_std_17)

# # Ensure the Transport DLL is copied on Windows
# if(WIN32)
#     # Add dependencies on the Transport DLL copy targets
#     if(TARGET copy_transport_dll_debug)
#         add_dependencies(${TARGETNAME} copy_transport_dll_debug)
#     endif()
#     if(TARGET copy_transport_dll_release)
#         add_dependencies(${TARGETNAME} copy_transport_dll_release)
#     endif()
# endif()

# Additional links
if(WIN32)
    target_link_libraries(${TARGETNAME} PRIVATE setupapi) # Windows additionally requires setupapi
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

# # Copy required DLLs to the executable directory on Windows
# if(WIN32)
#     set(OpenCV_BIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ImgProcesser/third_party/opencv/windows/x64/vc17/bin")

#     # Define the OpenCV DLL files to copy (without suffix)
#     set(OpenCV_DLL_NAMES
#         "opencv_core4120"
#         "opencv_imgcodecs4120"
#         "opencv_imgproc4120"
#     )

#     # Create a copy command for each OpenCV DLL
#     foreach(dll_name ${OpenCV_DLL_NAMES})
#         add_custom_command(TARGET ${TARGETNAME} POST_BUILD
#             COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             "${OpenCV_BIN_PATH}/${dll_name}$<$<CONFIG:Debug>:d>.dll"
#             "$<TARGET_FILE_DIR:${TARGETNAME}>/${dll_name}$<$<CONFIG:Debug>:d>.dll"
#             COMMENT "📦 Copying ${dll_name}$<$<CONFIG:Debug>:d>.dll to output directory"
#         )
#     endforeach()
# endif()
if(WIN32)
add_custom_command(TARGET main POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:main> $<TARGET_FILE_DIR:main>
COMMAND_EXPAND_LISTS)
endif()
