cmake_minimum_required(VERSION 3.20)

include(FetchContent)

project(vi-rt)

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(CXX_STANDARD C++20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fno-fast-math -fsanitize=address -fopenmp")

file(GLOB_RECURSE sources src/**/*.cpp src/*.cpp include/*.hpp)

add_subdirectory(vendor/fmt)
add_subdirectory(vendor/tinyobjloader)
add_subdirectory(vendor/glm)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(json)

add_executable(vi-rt ${sources} include/helpers.hpp)

find_package(OpenMP)
if (OpenMP_CXX_FOUND)
    target_link_libraries(vi-rt OpenMP::OpenMP_CXX)
endif ()

set(ZLIB_LIBRARY ENV{ZLIB_LIBRARY})
set(ZLIB_INCLUDE_DIR ENV{ZLIB_INCLUDE_DIR})

find_package(OpenImageIO REQUIRED)

target_link_libraries(vi-rt nlohmann_json::nlohmann_json)
target_link_libraries(vi-rt fmt::fmt)
target_link_libraries(vi-rt tinyobjloader)
target_link_libraries(vi-rt OpenImageIO::OpenImageIO)
target_link_libraries(vi-rt glm)
target_link_libraries(vi-rt m)

target_include_directories(vi-rt PRIVATE src include)
target_include_directories(vi-rt PRIVATE ${OIIO_INCLUDE_DIRS})
