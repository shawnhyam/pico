cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(ili9341)

pico_sdk_init()

add_executable(ili9341
    ili9341.c
)

pico_enable_stdio_usb(ili9341 1)
# pico_enable_stdio_uart(test 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(ili9341)

target_link_libraries(ili9341 pico_stdlib hardware_spi)
