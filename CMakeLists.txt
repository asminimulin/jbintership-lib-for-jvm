cmake_minimum_required (VERSION 3.17)

project ("JBIntership")

# Helpers
if (MSVC)
    include("${CMAKE_SOURCE_DIR}/cmake/WindowsMacro.cmake")
endif(MSVC)


set(SRCS main.cpp "application.cpp" "main_window.cpp")
set(HEADERS application.hpp "forward_definitions.hpp" "log_stream.hpp" "main_window.hpp")
add_executable (Application  ${SRCS} ${HEADERS})

target_link_libraries(Application d2d1)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Application PRIVATE "-DDEBUG")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
