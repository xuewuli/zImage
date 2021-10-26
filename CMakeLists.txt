cmake_minimum_required(VERSION 3.5)
project(zImage)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -march=native")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
endif()

set(CMAKE_CXX_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(
  zimg
    zpng.cpp
    zpng.h
    zpvr.cpp
    zpvr.h
    zimg_app.cpp
)

target_include_directories(
  zimg
    PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/
)

include(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zstd/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/zlib/CMakeLists.txt)

target_link_libraries(zimg libzstd_static zlib Threads::Threads)

