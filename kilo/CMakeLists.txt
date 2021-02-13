add_executable(kilo
    kilo.c
	kilo_platform_mode0.c
)

# add_definitions("-Wall -Wextra -pedantic -std=c99")

pico_enable_stdio_usb(kilo 1)
pico_add_extra_outputs(kilo)
target_link_libraries(kilo mode0 pico_stdlib pico_mem_ops pico_malloc hardware_spi hardware_flash)


add_executable(kilo_ssd1306
	kilo.c
	kilo_platform_ssd1306.c
)

pico_enable_stdio_usb(kilo_ssd1306 1)
pico_add_extra_outputs(kilo_ssd1306)
target_link_libraries(kilo_ssd1306 ssd1306 pico_stdlib pico_mem_ops pico_malloc hardware_i2c hardware_flash)

