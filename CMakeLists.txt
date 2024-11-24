cmake_minimum_required(VERSION 3.20)
project(WoW_335a_Patcher)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(WoW_335a_Patcher main.cpp)

# MinGW specific static linking to avoid runtime DLL dependencies
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

# Enable verbose makefile for debugging (optional)
set(CMAKE_VERBOSE_MAKEFILE ON)
