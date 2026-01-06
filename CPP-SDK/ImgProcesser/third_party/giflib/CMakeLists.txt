cmake_minimum_required(VERSION 3.12)

project(giflib LANGUAGES C)

# Option: build tools (optional)
option(GIFLIB_BUILD_TOOLS "Build giflib CLI tools (gif2rgb, etc.)" OFF)

# Set library version
set(GIFLIB_VERSION_MAJOR 7)
set(GIFLIB_VERSION_MINOR 2)
set(GIFLIB_VERSION_PATCH 0)
set(GIFLIB_VERSION "${GIFLIB_VERSION_MAJOR}.${GIFLIB_VERSION_MINOR}.${GIFLIB_VERSION_PATCH}")

# Source files
set(GIFLIB_SRCS
    dgif_lib.c
    egif_lib.c
    gifalloc.c
    gif_err.c
    gif_font.c
    gif_hash.c
    openbsd-reallocarray.c
)

# Header files
set(GIFLIB_PUBLIC_HEADERS
    gif_lib.h
)

# giflib static library
add_library(gif_lib STATIC ${GIFLIB_SRCS})
add_library(giflib::gif ALIAS gif_lib)

target_include_directories(gif_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_definitions(gif_lib PRIVATE LIBGIF_STATIC)

set_target_properties(gif_lib PROPERTIES
    VERSION ${GIFLIB_VERSION}
    PUBLIC_HEADER "${GIFLIB_PUBLIC_HEADERS}"
)

# Tools (optional)
if(GIFLIB_BUILD_TOOLS)
    set(UTIL_SRCS
        qprintf.c quantize.c getarg.c
    )
    set(UTIL_HDRS getarg.h)

    add_library(gifutil STATIC ${UTIL_SRCS})
    target_include_directories(gifutil PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    set(GIFLIB_TOOLS
        gif2rgb gifbuild giffix giftext giftool gifclrmp
        gifbg gifcolor gifecho giffilter gifhisto gifinto gifsponge gifwedge
    )

    foreach(tool ${GIFLIB_TOOLS})
        add_executable(${tool} ${tool}.c)
        target_link_libraries(${tool} gif_lib gifutil m)
    endforeach()
endif()
