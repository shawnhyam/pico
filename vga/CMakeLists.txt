add_executable(vga
    vga.c
)

target_link_libraries(vga 
	pico_stdlib
    hardware_pio
	hardware_irq
	hardware_dma
)


pico_generate_pio_header(vga 
	${CMAKE_CURRENT_LIST_DIR}/vga_sync.pio
)


# add_definitions("-Wall -Wextra -pedantic -std=c99")

target_compile_definitions(vga PRIVATE
	PICO_DEFAULT_UART=1
	PICO_DEFAULT_UART_TX_PIN=20
	PICO_DEFAULT_UART_RX_PIN=21
)

pico_add_extra_outputs(vga)


# target_link_libraries(${PROJECT_NAME}
#         pico_stdlib
#         hardware_pio
# 	hardware_dma
#         hardware_irq
# 	hardware_structs
#         )
