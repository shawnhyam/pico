
add_executable(vga
    vga.c
)

target_compile_definitions(vga PRIVATE
    PICO_SCANVIDEO_PLANE_COUNT=3

    PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=360
    PICO_SCANVIDEO_MAX_SCANLINE_BUFFER2_WORDS=360
    PICO_SCANVIDEO_MAX_SCANLINE_BUFFER3_WORDS=360

    # PICO_SCANVIDEO_PLANE1_FIXED_FRAGMENT_DMA=1
    # PICO_SCANVIDEO_PLANE2_FIXED_FRAGMENT_DMA=1
    # PLANE3 is not allowed to use DMA

    # Unclear if we need any buffering or not...
    # PICO_SCANVIDEO_SCANLINE_BUFFER_COUNT=4

	PICO_DEFAULT_UART=1
	PICO_DEFAULT_UART_TX_PIN=20
	PICO_DEFAULT_UART_RX_PIN=21
)

target_link_libraries(vga PRIVATE
    pico_multicore
    pico_stdlib
    pico_scanvideo_dpi
    hardware_dma
)

pico_add_extra_outputs(vga)
