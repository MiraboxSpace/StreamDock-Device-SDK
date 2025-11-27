option(TRANSPORTDLL_USE_IMPORTED "Use prebuilt transport library instead of building from source" OFF)

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

if(NOT TRANSPORTDLL_USE_IMPORTED)
    message(STATUS "BUILD TRANSPORT LIBRARY WITH SOURCE CODE")
    set(TRANSPORT_TRAGET "transport${TRANSPORT_ARCH_SUFFIX}")

    add_library(TransportDLL SHARED
        transport_c.cpp
        transport_cpp.cpp
    )

    set_target_properties(TransportDLL PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        OUTPUT_NAME "${TRANSPORT_TRAGET}"
    )

    target_include_directories(TransportDLL
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(TransportDLL PRIVATE hidapi::hidapi)

    target_compile_features(TransportDLL PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_definitions(TransportDLL PRIVATE TRANSPORTDLL_EXPORTS) # 导出函数设置
        target_compile_definitions(TransportDLL PRIVATE _AMD64_)
        set_target_properties(TransportDLL PROPERTIES
            OUTPUT_NAME_DEBUG "transportd${TRANSPORT_ARCH_SUFFIX}"
            OUTPUT_NAME_RELEASE "transport${TRANSPORT_ARCH_SUFFIX}"
        )
        # 从源码编译时，DLL会自动输出到 CMAKE_RUNTIME_OUTPUT_DIRECTORY
        # 但为了确保DLL在正确位置，添加额外的复制命令
        add_custom_command(TARGET TransportDLL POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:TransportDLL>"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>}/$<TARGET_FILE_NAME:TransportDLL>"
            COMMENT "📦 Copying built $<TARGET_FILE_NAME:TransportDLL> to bin/$<CONFIG>"
        )
    elseif(APPLE)
        set_target_properties(TransportDLL PROPERTIES
            INSTALL_RPATH "@loader_path;@executable_path/../lib"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
        find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
        find_library(IOKIT_FRAMEWORK IOKit)
        find_library(APPLICATIONSERVICES_FRAMEWORK ApplicationServices)

        target_link_libraries(TransportDLL PRIVATE
            ${COREFOUNDATION_FRAMEWORK}
            ${IOKIT_FRAMEWORK}
            ${APPLICATIONSERVICES_FRAMEWORK}
        )
    elseif(UNIX AND NOT APPLE)
        # Linux下需要确保符号被正确导出
        target_compile_options(TransportDLL PRIVATE -fvisibility=hidden)
        target_compile_definitions(TransportDLL PRIVATE TRANSPORTDLL_EXPORTS)
        set_target_properties(TransportDLL PROPERTIES
            LINK_FLAGS "-Wl,--no-undefined"
        )
        # 添加 pthread 库链接
        find_package(Threads REQUIRED)
        target_link_libraries(TransportDLL PRIVATE Threads::Threads)
    endif()

else()
    message(STATUS "BUILD TRANSPORT LIBRARY WITH LIBRARY FILE")
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
            IMPORTED_IMPLIB_DEBUG "${CMAKE_CURRENT_LIST_DIR}/transportd${TRANSPORT_ARCH_SUFFIX}.lib"
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/transportd${TRANSPORT_ARCH_SUFFIX}.dll"
            IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_LIST_DIR}/transport${TRANSPORT_ARCH_SUFFIX}.lib"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/transport${TRANSPORT_ARCH_SUFFIX}.dll"
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}"
        )
        # 拷贝已编译的 transport 动态库到运行目录（支持分离的 Debug/Release 目录）
        # Debug 配置
        add_custom_target(copy_transport_dll_debug
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/Debug"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_LIST_DIR}/transportd${TRANSPORT_ARCH_SUFFIX}.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/Debug/transportd${TRANSPORT_ARCH_SUFFIX}.dll"
            COMMENT "📦 Copying prebuilt transportd${TRANSPORT_ARCH_SUFFIX}.dll to bin/Debug"
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

endif()
