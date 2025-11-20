include(FetchContent)

# 设置 hidapi 编译为动态库
set(BUILD_SHARED_LIBS ${USE_SHARED_HIDAPI})

# 设置 hidapi 输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

FetchContent_Declare(
    hidapi
    URL "${CMAKE_SOURCE_DIR}/third_party/hidapi-master.zip"
    # URL_HASH "SHA256=e0a2f8fdaddba3e63f7d47336a584ed416750d55083c0ee0a474e55adb6027d1"
    # DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(hidapi)
