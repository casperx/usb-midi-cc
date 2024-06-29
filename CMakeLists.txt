cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(usb-midi-expression C CXX ASM)

pico_sdk_init()

set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)

add_subdirectory(src out)

pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/link.ld)
