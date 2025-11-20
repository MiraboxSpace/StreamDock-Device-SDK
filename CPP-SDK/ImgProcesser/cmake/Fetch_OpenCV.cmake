# === Fetch_OpenCV.cmake ===
# 导入解压模块, 解压源码到目标路径, 使用源码进行编译

if(APPLE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ExtractHelpers.cmake)
    extract_archive_flat(
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv.zip"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv"
    )
    set(OpenCV_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv/mac)
    set(OpenCV_INCLUDE_DIR ${OpenCV_ROOT}/include/opencv4)
    # 自动收集所有动态库（.dylib）
    file(GLOB OpenCV_LIBS
        ${OpenCV_ROOT}/lib/libopencv_core*.dylib
        ${OpenCV_ROOT}/lib/libopencv_imgproc*.dylib
        ${OpenCV_ROOT}/lib/libopencv_imgcodecs*.dylib
    )
    # 创建 OpenCV 接口库 target
    add_library(OpenCV INTERFACE)
    target_include_directories(OpenCV INTERFACE ${OpenCV_INCLUDE_DIR})
    target_link_libraries(OpenCV INTERFACE ${OpenCV_LIBS})
elseif(WIN32)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ExtractHelpers.cmake)
    extract_archive_flat(
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv.zip"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv"
    )
    set(OpenCV_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv/windows)
    set(OpenCV_DIR ${OpenCV_ROOT}/x64/vc17/lib)
    find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

    # 显示找到的 OpenCV 信息（调试用）
    message(STATUS "OpenCV version: ${OpenCV_VERSION}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
elseif(UNIX AND NOT APPLE)
    find_package(OpenCV REQUIRED)
endif()

