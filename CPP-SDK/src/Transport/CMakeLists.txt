set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 为 MSVC 设置运行时库 - 确保与主项目一致
if(MSVC)
    # 强制使用动态运行时库
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

# 添加 TransportDLL 子模块（保持源码结构）
add_subdirectory(TransportDLL)

# TransportCWrapper 使用 TransportDLL 提供的 API
add_library(TransportCWrapper STATIC TransportCWrapper.cpp)

# 链接 C wrapper 动态库 和 hidapi 库Add commentMore actions
target_link_libraries(TransportCWrapper
    PUBLIC hidapi::hidapi
    PRIVATE TransportDLL
)

target_include_directories(TransportCWrapper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/TransportDLL
)
