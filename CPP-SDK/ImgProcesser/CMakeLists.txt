cmake_minimum_required(VERSION 3.16)
project(img_processer)

set(CMAKE_CXX_STANDARD 17)

# 为 MSVC 设置运行时库
if(MSVC)
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    # 强制设置运行时库标志
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
        string(REPLACE "/MDd" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MD" "" ${flag_var} "${${flag_var}}")
    endforeach()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif()

include(cmake/Fetch_giflib.cmake)
# 导入 giflib 和 opencv
if(WIN32)
    # windows 时只能直接使用源码编译, 已在 Fetch_giflib.cmake 中解压了
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

# 构建库：ImgProcesser
add_library(ImgProcesser STATIC
    ${Gif2Jpg}/Gif2ImgFrame.cpp
    ${OpenCVEncoder}/OpenCVImageEncoder.cpp
)

# 头文件路径（对外暴露）
target_include_directories(ImgProcesser PUBLIC
    ${Gif2Jpg}
    ${OpenCVEncoder}
    ${ImgHelper}
)

# 链接依赖
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

# Windows预编译OpenCV库文件拷贝
if(WIN32)
    set(OpenCV_LIB_PATH_PREFIX "${CMAKE_CURRENT_LIST_DIR}/third_party/opencv/windows/x64/vc17/bin")

    # 定义需要复制的 DLL 文件（不带后缀）
    set(OpenCV_DLL_NAMES
        "opencv_core4120"
        "opencv_imgcodecs4120"
        "opencv_imgproc4120"
    )

    # 为每个 DLL 创建复制命令
    foreach(dll_name ${OpenCV_DLL_NAMES})
        add_custom_command(TARGET ImgProcesser POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OpenCV_LIB_PATH_PREFIX}/${dll_name}$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:ImgProcesser>/../bin/$<CONFIG>/${dll_name}$<$<CONFIG:Debug>:d>.dll"
            COMMENT "📦 Copying ${dll_name}$<$<CONFIG:Debug>:d>.dll to $<CONFIG> output directory"
        )
    endforeach()
endif()
