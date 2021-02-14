add_library(ili9341
	font.c
	ili9341.c
)
target_link_libraries(ili9341 pico_stdlib hardware_spi)
target_include_directories(ili9341 PUBLIC include)


# MODE 1 (SPRITE GRAPHICS)
add_library(mode1
	mode1.c
)

target_link_libraries(mode1 ili9341 pico_stdlib hardware_spi)
target_include_directories(mode1 PUBLIC include)

add_executable(mode1_demo
    mode1_demo.c
)

pico_enable_stdio_usb(mode1_demo 1)
pico_add_extra_outputs(mode1_demo)
target_link_libraries(mode1_demo mode1 pico_stdlib hardware_spi)


# MODE 2 (FRAMEBUFFER)
add_library(mode2
	mode2.c
)

target_link_libraries(mode2 ili9341 pico_stdlib hardware_spi)
target_include_directories(mode2 PUBLIC include)

add_executable(mode2_demo
	mode2_demo.c
)

pico_enable_stdio_usb(mode2_demo 1)
pico_add_extra_outputs(mode2_demo)
target_link_libraries(mode2_demo mode2 pico_stdlib hardware_spi)


# MODE 0 (TEXT)
add_library(mode0
	mode0.c
)
target_link_libraries(mode0 ili9341 pico_stdlib hardware_spi)
target_include_directories(mode0 PUBLIC include)

add_executable(mode0_demo
    mode0_demo.c
)

pico_enable_stdio_usb(mode0_demo 1)
pico_add_extra_outputs(mode0_demo)
target_link_libraries(mode0_demo mode0 pico_stdlib pico_mem_ops hardware_spi)
