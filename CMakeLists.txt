cmake_minimum_required (VERSION 3.17)

project ("JBIntership")

# Helpers
if (MSVC)
    include("${CMAKE_SOURCE_DIR}/cmake/WindowsMacro.cmake")
endif(MSVC)


set(SRCS main.cpp "application.cpp")
set(HEADERS application.hpp "forward_definitions.hpp" "log_stream.hpp")
add_executable (Application  ${SRCS} ${HEADERS})

target_link_libraries(Application d2d1)
