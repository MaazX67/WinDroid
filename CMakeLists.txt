cmake_minimum_required(VERSION 3.24)
project(APKRunner-Universal VERSION 0.1.0 LANGUAGES CXX)

option(APKRUNNER_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(libzip CONFIG QUIET)
if(TARGET libzip::zip)
    set(APKRUNNER_LIBZIP_TARGET libzip::zip)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBZIP REQUIRED IMPORTED_TARGET libzip)
    set(APKRUNNER_LIBZIP_TARGET PkgConfig::LIBZIP)
endif()

add_library(apkrunner_core STATIC
    src/core/apk_parser.cpp
    src/core/elf_loader.cpp
    src/engines/detector.cpp
)
target_include_directories(apkrunner_core PUBLIC include)
target_link_libraries(apkrunner_core PRIVATE ${APKRUNNER_LIBZIP_TARGET})

add_executable(APKRunner src/cli/main.cpp)
target_link_libraries(APKRunner PRIVATE apkrunner_core)

if(MSVC)
    target_compile_options(apkrunner_core PRIVATE /W4)
    target_compile_options(APKRunner PRIVATE /W4)
    if(APKRUNNER_WARNINGS_AS_ERRORS)
        target_compile_options(apkrunner_core PRIVATE /WX)
        target_compile_options(APKRunner PRIVATE /WX)
    endif()
else()
    target_compile_options(apkrunner_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(APKRunner PRIVATE -Wall -Wextra -Wpedantic)
    if(APKRUNNER_WARNINGS_AS_ERRORS)
        target_compile_options(apkrunner_core PRIVATE -Werror)
        target_compile_options(APKRunner PRIVATE -Werror)
    endif()
endif()

install(TARGETS APKRunner RUNTIME DESTINATION bin)