# MODE 1
add_executable(mode1_demo
    font.c
    ili9341.c
    mode1_demo.c
)

pico_enable_stdio_usb(mode1_demo 1)
pico_add_extra_outputs(mode1_demo)
target_link_libraries(mode1_demo pico_stdlib hardware_spi)

# MODE 0 (TEXT)
add_executable(mode0_demo
    font.c
    ili9341.c
    mode0.c
    mode0_demo.c
)

pico_enable_stdio_usb(mode0_demo 1)
pico_add_extra_outputs(mode0_demo)
target_link_libraries(mode0_demo pico_stdlib pico_mem_ops hardware_spi)
