set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set the MSVC runtime library - keep consistent with the main project
if(MSVC)
    # Force the dynamic runtime library
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/MDd" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MTd" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MD" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/MT" "" ${flag_var} "${${flag_var}}")
    endforeach()

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MD")
endif()

# Add the TransportDLL submodule (keep source layout)
add_subdirectory(TransportDLL)

# TransportCWrapper uses the API provided by TransportDLL
add_library(TransportCWrapper STATIC TransportCWrapper.cpp)

# Link the C wrapper shared library and hidapi library
target_link_libraries(TransportCWrapper
    PUBLIC hidapi::hidapi
    PRIVATE TransportDLL
)

target_include_directories(TransportCWrapper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/TransportDLL
)
