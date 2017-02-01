cmake_minimum_required(VERSION 2.8)
project(snap-plugin-collector-rdt)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(STATUS "Error: ${CMAKE_CXX_COMPILER} has no C++11 support")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

include_directories(
        ${CMAKE_SOURCE_DIR}/include
)

set(SOURCE_FILES
        src/main.cpp
        src/rdt/rdt.cpp
        src/rdt/rdt_utils.cpp src/rdt/rdt_utils.hpp)

add_executable(snap-plugin-collector-rdt ${SOURCE_FILES})

target_link_libraries(snap-plugin-collector-rdt
        ${CMAKE_SOURCE_DIR}/lib/libpqos.a
        ${CMAKE_SOURCE_DIR}/lib/libsnap.a
        protobuf
        grpc++
        )
