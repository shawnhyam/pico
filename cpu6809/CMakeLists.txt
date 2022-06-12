add_library(cpu6809
	cpu.c
    main.c
)

target_compile_definitions(cpu6809 PRIVATE
)

target_link_libraries(cpu6809 
	pico_stdlib
)

target_include_directories(cpu6809 PUBLIC include)




add_executable(cpu6809_test
    cpu_test.c
)

# add_definitions("-Wall -Wextra -pedantic -std=c99")

target_compile_definitions(cpu6809_test PRIVATE
	PICO_DEFAULT_UART=1
	PICO_DEFAULT_UART_TX_PIN=20
	PICO_DEFAULT_UART_RX_PIN=21
)

pico_add_extra_outputs(cpu6809_test)

target_link_libraries(cpu6809_test 
	cpu6809 
)
