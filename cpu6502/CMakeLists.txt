add_library(cpu6502
	fake6502.c
	minimal_computer.c
)

target_compile_definitions(cpu6502 PRIVATE
)

target_link_libraries(cpu6502 
	pico_multicore
	pico_stdlib
	pico_scanvideo_dpi
	hardware_dma
	vga
)

target_include_directories(cpu6502 PUBLIC include)




add_executable(cpu6502_test
    cpu_test.c
)

# add_definitions("-Wall -Wextra -pedantic -std=c99")

target_compile_definitions(cpu6502_test PRIVATE
	PICO_DEFAULT_UART=1
	PICO_DEFAULT_UART_TX_PIN=20
	PICO_DEFAULT_UART_RX_PIN=21
)

# target_compile_options(cpu6502_test PRIVATE -Werror -Wall -Wextra -Wpedantic)

pico_add_extra_outputs(cpu6502_test)

target_link_libraries(cpu6502_test 
	cpu6502
	vga
)
