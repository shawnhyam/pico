cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(retro)

pico_sdk_init()

add_executable(retro
    repl.c
)

pico_enable_stdio_usb(retro 1)
# pico_enable_stdio_uart(retro 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(retro)

target_link_libraries(retro pico_stdlib pico_printf)
