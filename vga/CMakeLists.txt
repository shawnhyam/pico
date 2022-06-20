### VGA LIBRARY

add_library(vga
	vga.c
    mode0.c
    mode1.c
    mode2.c
    # modex.c
    modez.c
    fonts.c
)

target_link_libraries(vga PRIVATE
    pico_multicore
    pico_stdlib
    pico_scanvideo_dpi
    hardware_dma
)

target_include_directories(vga PUBLIC include)


target_compile_definitions(vga PUBLIC
    PICO_SCANVIDEO_PLANE_COUNT=1
    PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=360
)

### VGA_DEMO EXECUTABLE

add_executable(vga_demo
    vga_demo.c
)


target_compile_definitions(vga_demo PRIVATE
    PICO_SCANVIDEO_PLANE_COUNT=1

    PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=400
    PICO_SCANVIDEO_MAX_SCANLINE_BUFFER2_WORDS=360
    PICO_SCANVIDEO_MAX_SCANLINE_BUFFER3_WORDS=360

    # PICO_SCANVIDEO_PLANE1_FIXED_FRAGMENT_DMA=1
    # PICO_SCANVIDEO_PLANE2_FIXED_FRAGMENT_DMA=1
    # PLANE3 is not allowed to use DMA

    # Unclear if we need any buffering or not...
    # PICO_SCANVIDEO_SCANLINE_BUFFER_COUNT=8

	PICO_DEFAULT_UART=1
	PICO_DEFAULT_UART_TX_PIN=20
	PICO_DEFAULT_UART_RX_PIN=21
)

target_link_libraries(vga_demo PRIVATE
    pico_multicore
    pico_stdlib
    vga
)

pico_add_extra_outputs(vga_demo)
