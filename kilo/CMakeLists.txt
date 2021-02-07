cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(kilo)

pico_sdk_init()

add_executable(kilo
    ../ili9341/mode0.c
    ../ili9341/ili9341.c
    kilo.c
)

# add_definitions("-Wall -Wextra -pedantic -std=c99")

pico_enable_stdio_usb(kilo 1)
pico_add_extra_outputs(kilo)
target_link_libraries(kilo pico_stdlib pico_mem_ops pico_malloc hardware_spi)
