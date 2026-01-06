message(STATUS "BUILD TRANSPORT LIBRARY WITH LIBRARY FILE")

# Set the library name suffix by platform and architecture
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
    # Copy the built transport shared library to the runtime directory (supports separate Debug/Release dirs)
    # Debug config - use the release library
    add_custom_target(copy_transport_dll_debug
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/Debug"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_LIST_DIR}/transport${TRANSPORT_ARCH_SUFFIX}.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/Debug/transport${TRANSPORT_ARCH_SUFFIX}.dll"
        COMMENT "📦 Copying prebuilt transport${TRANSPORT_ARCH_SUFFIX}.dll to bin/Debug"
    )
    # Release config
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
    # Copy the built transport shared library to the runtime directory
    # Set the target path
    set(TRANSPORT_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/${TRANSPORT_LIB_FILENAME}")
    set(TRANSPORT_LIB_DEST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TRANSPORT_LIB_FILENAME}")
    # Only add the copy task when the target file does not exist
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
