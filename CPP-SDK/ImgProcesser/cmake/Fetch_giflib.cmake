# === Fetch_giflib.cmake ===
if (APPLE)
    set(GIFLIB_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps/giflib)
    include(ExternalProject)
    
    ExternalProject_Add(giflib
        URL ${CMAKE_CURRENT_SOURCE_DIR}/third_party/giflib-5.2.2.tar.gz
        PREFIX ${GIFLIB_PREFIX}
        DOWNLOAD_EXTRACT_TIMESTAMP true
        CONFIGURE_COMMAND ""                      # No ./configure step
        BUILD_COMMAND make                        # Build using the Makefile
        BUILD_IN_SOURCE 1                         # Build in the source directory
        INSTALL_COMMAND ""                        # Do not install into the system path
    )
    
    # Define an IMPORTED static library target
    add_library(gif_lib SHARED IMPORTED GLOBAL)
    add_dependencies(gif_lib giflib)
    
    # Set the path (Make generates libgif.a by default)
    set_target_properties(gif_lib PROPERTIES
        IMPORTED_LOCATION ${GIFLIB_PREFIX}/src/giflib/libgif.a
        INTERFACE_INCLUDE_DIRECTORIES ${GIFLIB_PREFIX}/src/giflib
    )
elseif(WIN32)
    # Import the extract helper, unpack sources to the target path, and build from source
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ExtractHelpers.cmake)
    extract_archive_flat(
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/giflib-5.2.2.tar.gz"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/giflib"
    )
endif()
