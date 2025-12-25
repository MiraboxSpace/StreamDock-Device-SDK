message(STATUS "BUILD TRANSPORT LIBRARY WITH LIBRARY FILE")

# 根据平台和架构设置库名称后缀
if(WIN32)
    set(TRANSPORT_ARCH_SUFFIX "")
elseif(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(TRANSPORT_ARCH_SUFFIX "_arm64")
    else()
        set(TRANSPORT_ARCH_SUFFIX "")
    endif()
elseif(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(TRANSPORT_ARCH_SUFFIX "_arm64")
    else()
        set(TRANSPORT_ARCH_SUFFIX "")
    endif()
endif()

add_library(TransportDLL SHARED IMPORTED GLOBAL)

if(WIN32)
    # do nothing
elseif(APPLE)
    set(TRANSPORT_LIB_FILENAME "libtransport${TRANSPORT_ARCH_SUFFIX}.dylib")
elseif(UNIX AND NOT APPLE)
    set(TRANSPORT_LIB_FILENAME "libtransport${TRANSPORT_ARCH_SUFFIX}.so")
endif()

if(WIN32)
    set_target_properties(TransportDLL PROPERTIES
        IMPORTED_IMPLIB_DEBUG "${CMAKE_CURRENT_LIST_DIR}/transport${TRANSPORT_ARCH_SUFFIX}.lib"
        IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/transport${TRANSPORT_ARCH_SUFFIX}.dll"
        IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_LIST_DIR}/transport${TRANSPORT_ARCH_SUFFIX}.lib"
        IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/transport${TRANSPORT_ARCH_SUFFIX}.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}"
    )
    # 拷贝已编译的 transport 动态库到运行目录（支持分离的 Debug/Release 目录）
    # Debug 配置 - 使用 release 版本的库
    add_custom_target(copy_transport_dll_debug
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/Debug"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_LIST_DIR}/transport${TRANSPORT_ARCH_SUFFIX}.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/Debug/transport${TRANSPORT_ARCH_SUFFIX}.dll"
        COMMENT "📦 Copying prebuilt transport${TRANSPORT_ARCH_SUFFIX}.dll to bin/Debug"
    )
    # Release 配置
    add_custom_target(copy_transport_dll_release
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/Release"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_LIST_DIR}/transport${TRANSPORT_ARCH_SUFFIX}.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/Release/transport${TRANSPORT_ARCH_SUFFIX}.dll"
        COMMENT "📦 Copying prebuilt transport${TRANSPORT_ARCH_SUFFIX}.dll to bin/Release"
    )
else()
    set_target_properties(TransportDLL PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/${TRANSPORT_LIB_FILENAME}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}"
    )
    # 拷贝已编译的 transport 动态库到运行目录
    # 设置目标路径
    set(TRANSPORT_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/${TRANSPORT_LIB_FILENAME}")
    set(TRANSPORT_LIB_DEST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TRANSPORT_LIB_FILENAME}")
    # 只有当目标文件不存在时，才添加 copy 任务
    if(NOT EXISTS "${TRANSPORT_LIB_DEST}")
        add_custom_target(copy_transport_dll ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${TRANSPORT_LIB_PATH}"
            "${TRANSPORT_LIB_DEST}"
            COMMENT "📦 Copying prebuilt ${TRANSPORT_LIB_FILENAME} to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        )
    else()
        message(STATUS "✅ Skipped copy: ${TRANSPORT_LIB_FILENAME} already exists in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()
endif()
