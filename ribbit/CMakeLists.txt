add_executable(ribbit
    ribbit.c
)

# add_definitions("-Wall -Wextra -pedantic -std=c99")

target_compile_definitions(ribbit PRIVATE
	PICO_DEFAULT_UART=1
	PICO_DEFAULT_UART_TX_PIN=20
	PICO_DEFAULT_UART_RX_PIN=21
)

pico_add_extra_outputs(ribbit)
target_link_libraries(ribbit pico_stdlib)
