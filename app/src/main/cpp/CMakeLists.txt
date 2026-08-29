cmake_minimum_required(VERSION 3.22.1)
project("windroid")

# Set C++ standard to C++17 for WinCore
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable all warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# WinCore library - Main Windows emulation engine
add_library(wincore SHARED
    wincore/wincore.cpp
    wincore/wincore.h
    wincore/jit_translator.cpp
    wincore/jit_translator.h
    wincore/api_layer.cpp
    wincore/api_layer.h
)

# Include directories for WinCore
target_include_directories(wincore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/wincore
)

# Link Android log library
target_link_libraries(wincore PRIVATE log)

# Original native_loader library
add_library(windroid SHARED native_loader.c)

# Link windroid to wincore so JNI can access it
target_link_libraries(windroid PRIVATE wincore log)
