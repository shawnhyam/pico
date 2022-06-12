add_library(coco4
	coco4.c
	video.c
	sound.c
)

target_compile_definitions(coco4 PRIVATE
	PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=360
	PICO_SCANVIDEO_SCANLINE_BUFFER_COUNT=4
	PICO_SCANVIDEO_PLANE1_FIXED_FRAGMENT_DMA=1
)

target_link_libraries(coco4 
	pico_stdlib
	pico_multicore
	pico_scanvideo_dpi
	pico_audio_pwm
	hardware_dma
)
target_include_directories(coco4 PUBLIC include)




add_executable(coco4_demo
    coco4_demo.c
)

# add_definitions("-Wall -Wextra -pedantic -std=c99")

target_compile_definitions(coco4_demo PRIVATE
	PICO_DEFAULT_UART=1
	PICO_DEFAULT_UART_TX_PIN=20
	PICO_DEFAULT_UART_RX_PIN=21
)

pico_add_extra_outputs(coco4_demo)
target_link_libraries(coco4_demo 
	coco4 
	pico_stdlib
)
