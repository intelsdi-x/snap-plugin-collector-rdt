cmake_minimum_required(VERSION 2.8)
project(snap-plugin-collector-rdt)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)

#message("Using Clang 3.5 as CXX compiler")
#set(CMAKE_C_COMPILER "/usr/bin/clang")
#set(CMAKE_C_FLAGS  "-Wall -std=c99")
#set(CMAKE_C_FLAGS_DEBUG   "-g")
#set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")
#set(CMAKE_C_FLAGS_RELEASE   "-O4 -DNDEBUG")
#set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
#set(CMAKE_CXX_COMPILER "/usr/bin/clang")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
#set(EXTRA_LIBRARIES "stdc++ m")

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(STATUS "Error: ${CMAKE_CXX_COMPILER} has no C++11 support")
endif()

include_directories(
        ${CMAKE_SOURCE_DIR}/include
)

set(SOURCE_FILES
        src/main.cpp
        src/rdt/rdt.cpp
        )

add_executable(snap-plugin-collector-rdt ${SOURCE_FILES})

target_link_libraries(snap-plugin-collector-rdt
        ${CMAKE_SOURCE_DIR}/lib/libpqos.a
        ${CMAKE_SOURCE_DIR}/lib/libsnap.a
        protobuf
        grpc++
        )
