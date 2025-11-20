# === Fetch_giflib.cmake ===
if (APPLE)
    set(GIFLIB_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps/giflib)
    include(ExternalProject)
    
    ExternalProject_Add(giflib
        URL ${CMAKE_CURRENT_SOURCE_DIR}/third_party/giflib-5.2.2.tar.gz
        PREFIX ${GIFLIB_PREFIX}
        DOWNLOAD_EXTRACT_TIMESTAMP true
        CONFIGURE_COMMAND ""                      # 没有 ./configure
        BUILD_COMMAND make                        # 使用 Makefile 构建
        BUILD_IN_SOURCE 1                         # 在源码目录中构建
        INSTALL_COMMAND ""                        # 不安装到系统路径
    )
    
    # 定义 IMPORTED 静态库目标
    add_library(gif_lib SHARED IMPORTED GLOBAL)
    add_dependencies(gif_lib giflib)
    
    # 设置路径（Make 默认生成 libgif.a）
    set_target_properties(gif_lib PROPERTIES
        IMPORTED_LOCATION ${GIFLIB_PREFIX}/src/giflib/libgif.a
        INTERFACE_INCLUDE_DIRECTORIES ${GIFLIB_PREFIX}/src/giflib
    )
elseif(WIN32)
    # 导入解压模块, 解压源码到目标路径, 使用源码进行编译
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ExtractHelpers.cmake)
    extract_archive_flat(
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/giflib-5.2.2.tar.gz"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/giflib"
    )
endif()
