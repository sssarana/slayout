cmake_minimum_required(VERSION 3.16)
project(slayoutc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES
    src/*.cpp
)

add_executable(slayoutc ${SOURCES})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(slayoutc PRIVATE -Wall -Wextra -pedantic)
elseif (MSVC)
    target_compile_options(slayoutc PRIVATE /W4)
endif()
