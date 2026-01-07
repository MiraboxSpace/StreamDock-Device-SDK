cmake_minimum_required(VERSION 3.16)
project(img_processer)

set(CMAKE_CXX_STANDARD 17)

# Set the MSVC runtime library
if(MSVC)
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    # Force the runtime library flags
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
        string(REPLACE "/MDd" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MD" "" ${flag_var} "${${flag_var}}")
    endforeach()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif()

include(cmake/Fetch_giflib.cmake)
# Import giflib and opencv
if(WIN32)
    # On Windows, build from source only; sources are already extracted in Fetch_giflib.cmake
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/giflib)
endif()
include(cmake/Fetch_OpenCV.cmake)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(Gif2Jpg ${SRC_DIR}/Gif2ImgFrame)
set(OpenCVEncoder ${SRC_DIR}/OpenCVImageEncoder)
set(ImgHelper ${SRC_DIR}/ImgHelper)
# add_executable(test_gif main.cpp ${Gif2Jpg}/Gif2ImgFrame.cpp ${SRC_DIR}/OpenCVImageEncoder/OpenCVImageEncoder.cpp)
# if(WIN32) 
#     target_link_libraries(test_gif PRIVATE gif_lib ${OpenCV_LIBS})
#     target_include_directories(test_gif PRIVATE ${Gif2Jpg} ${SRC_DIR}/OpenCVImageEncoder ${OpenCV_INCLUDE_DIRS})
# elseif(APPLE)
#     target_link_libraries(test_gif PRIVATE gif_lib OpenCV)
#     target_include_directories(test_gif PRIVATE ${Gif2Jpg} ${SRC_DIR}/OpenCVImageEncoder)
# endif()

# Build library: ImgProcesser
add_library(ImgProcesser STATIC
    ${Gif2Jpg}/Gif2ImgFrame.cpp
    ${OpenCVEncoder}/OpenCVImageEncoder.cpp
)

# Header include paths (public)
target_include_directories(ImgProcesser PUBLIC
    ${Gif2Jpg}
    ${OpenCVEncoder}
    ${ImgHelper}
)

# Link dependencies
if(WIN32)
    target_link_libraries(ImgProcesser PUBLIC gif_lib ${OpenCV_LIBS})
    target_include_directories(ImgProcesser PUBLIC ${OpenCV_INCLUDE_DIRS})
elseif(APPLE)
    target_link_libraries(ImgProcesser PUBLIC gif_lib OpenCV)
elseif(UNIX AND NOT APPLE)
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
    target_link_libraries(ImgProcesser PUBLIC gif ${OpenCV_LIBS})
    target_include_directories(ImgProcesser PUBLIC ${OpenCV_INCLUDE_DIRS})
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
endif()

# Copy prebuilt OpenCV libraries on Windows
if(WIN32)
    set(OpenCV_LIB_PATH_PREFIX "${CMAKE_CURRENT_LIST_DIR}/third_party/opencv/windows/x64/vc17/bin")

    # Define the DLL files to copy (without suffix)
    set(OpenCV_DLL_NAMES
        "opencv_core4120"
        "opencv_imgcodecs4120"
        "opencv_imgproc4120"
    )

    # Create a copy command for each DLL
    foreach(dll_name ${OpenCV_DLL_NAMES})
        add_custom_command(TARGET ImgProcesser POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OpenCV_LIB_PATH_PREFIX}/${dll_name}$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:ImgProcesser>/../bin/$<CONFIG>/${dll_name}$<$<CONFIG:Debug>:d>.dll"
            COMMENT "📦 Copying ${dll_name}$<$<CONFIG:Debug>:d>.dll to $<CONFIG> output directory"
        )
    endforeach()
endif()
