# === Fetch_OpenCV.cmake ===
# Import the extract helper, unpack sources to the target path, and build from source

if(APPLE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ExtractHelpers.cmake)
    extract_archive_flat(
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv.zip"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv"
    )
    set(OpenCV_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/opencv/mac)
    set(OpenCV_INCLUDE_DIR ${OpenCV_ROOT}/include/opencv4)
    # Automatically collect all dynamic libraries (.dylib)
    file(GLOB OpenCV_LIBS
        ${OpenCV_ROOT}/lib/libopencv_core*.dylib
        ${OpenCV_ROOT}/lib/libopencv_imgproc*.dylib
        ${OpenCV_ROOT}/lib/libopencv_imgcodecs*.dylib
    )
    # Create the OpenCV interface library target
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

    # Show the discovered OpenCV info (debug only)
    message(STATUS "OpenCV version: ${OpenCV_VERSION}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
elseif(UNIX AND NOT APPLE)
    find_package(OpenCV REQUIRED)
endif()

