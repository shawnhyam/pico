add_library(ssd1306
	ssd1306.c
	font.c
)

target_link_libraries(ssd1306 pico_stdlib hardware_i2c)
target_include_directories(ssd1306 PUBLIC include)

add_executable(ssd1306_demo
    ssd1306_demo.c
)

# pico_enable_stdio_usb(test 1)
# pico_enable_stdio_uart(test 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(ssd1306_demo)

target_link_libraries(ssd1306_demo ssd1306 pico_stdlib hardware_i2c)
