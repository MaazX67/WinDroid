cmake_minimum_required(VERSION 3.24)
project(APKRunner-Fusion VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(APKRunner-Fusion src/launcher/main.cpp)

if(MSVC)
    target_compile_options(APKRunner-Fusion PRIVATE /W4)
else()
    target_compile_options(APKRunner-Fusion PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32 AND MINGW)
    target_link_options(APKRunner-Fusion PRIVATE -static -static-libgcc -static-libstdc++)
endif()

install(TARGETS APKRunner-Fusion RUNTIME DESTINATION bin)