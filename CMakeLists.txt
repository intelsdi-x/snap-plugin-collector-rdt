cmake_minimum_required(VERSION 2.8)
project(snap-plugin-collector-rdt)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(FATAL_ERROR "Error: ${CMAKE_CXX_COMPILER} has no C++11 support")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

# Add coverage data
if(DEFINED ENV{TEST_TYPE})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
endif()

# Include Protobuf
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

# Include GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/snap
        ${CMAKE_SOURCE_DIR}/third_party/intel-cmt-cat/lib
)

set(SOURCE_FILES
        src/rdt/rdt.cpp
        src/rdt/rdt_utils.cpp
        )

add_executable(snap-plugin-collector-rdt src/main.cpp ${SOURCE_FILES})

# Unit tests
add_executable(test-medium src/medium_test.cpp ${SOURCE_FILES})

target_link_libraries(snap-plugin-collector-rdt
        ${CMAKE_SOURCE_DIR}/lib/libpqos.a
        ${CMAKE_SOURCE_DIR}/lib/libsnap.a
        /usr/local/lib/libprotoc.a
        /usr/local/lib/libprotobuf.a
        /usr/local/lib/libgrpc++.a
        /usr/local/lib/libgrpc.a
        pthread
        )

target_link_libraries(test-medium
        ${CMAKE_SOURCE_DIR}/lib/libpqos.a
        ${CMAKE_SOURCE_DIR}/lib/libsnap.a
        ${CMAKE_SOURCE_DIR}/lib/libgmock.a
        ${CMAKE_SOURCE_DIR}/lib/libgtest.a
        /usr/local/lib/libprotoc.a
        /usr/local/lib/libprotobuf.a
        /usr/local/lib/libgrpc++.a
        /usr/local/lib/libgrpc.a
        pthread
        )
