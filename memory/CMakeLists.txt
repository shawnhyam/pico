cmake_minimum_required(VERSION 3.12)

# 
add_library(allocate
	allocate.c
)

target_link_libraries(allocate pico_stdlib pico_mem_ops)
target_include_directories(allocate PUBLIC include)

add_executable(allocate_demo
    allocate_demo.c
)

pico_enable_stdio_usb(allocate_demo 1)
pico_add_extra_outputs(allocate_demo)
target_link_libraries(allocate_demo allocate pico_stdlib)

