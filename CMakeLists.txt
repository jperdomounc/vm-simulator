cmake_minimum_required(VERSION 3.15)
project(VirtualMemorySimulator VERSION 1.0 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()


include_directories(${PROJECT_SOURCE_DIR}/include)


set(SOURCES
    src/main.cpp
    src/TLB.cpp
    src/PageTable.cpp
    src/PhysicalMemory.cpp
    src/VirtualMemoryManager.cpp
)


add_executable(vm_simulator ${SOURCES})


enable_testing()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
