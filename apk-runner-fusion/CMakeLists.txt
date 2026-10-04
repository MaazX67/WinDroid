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
    target_link_libraries(APKRunner-Fusion PRIVATE shell32 ole32 advapi32 uuid)
    target_link_options(APKRunner-Fusion PRIVATE -static -static-libgcc -static-libstdc++)
    foreach(ADB_FILE adb.exe AdbWinApi.dll AdbWinUsbApi.dll NOTICE.txt)
        add_custom_command(TARGET APKRunner-Fusion POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/adb/${ADB_FILE}"
                "$<TARGET_FILE_DIR:APKRunner-Fusion>/${ADB_FILE}"
        )
    endforeach()
endif()

install(TARGETS APKRunner-Fusion RUNTIME DESTINATION bin)
