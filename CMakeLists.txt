cmake_minimum_required(VERSION 2.8)
project(snap-plugin-collector-rdt)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(STATUS "Error: ${CMAKE_CXX_COMPILER} has no C++11 support")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -g")

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/snap
)

set(SOURCE_FILES
        src/rdt/rdt.cpp
        src/rdt/rdt_utils.cpp
        )

add_executable(snap-plugin-collector-rdt src/main.cpp ${SOURCE_FILES})

add_executable(test-rdt src/main_test.cpp ${SOURCE_FILES})

target_link_libraries(snap-plugin-collector-rdt
        ${CMAKE_SOURCE_DIR}/lib/libpqos.a
        ${CMAKE_SOURCE_DIR}/lib/libsnap.a
        protobuf
        grpc++
        )

target_link_libraries(test-rdt
        ${CMAKE_SOURCE_DIR}/lib/libpqos.a
        ${CMAKE_SOURCE_DIR}/lib/libsnap.a
        protobuf
        grpc++
        )
