

    add_executable(textmode
            textmode.c
            foo.c
      )

        add_compile_options(-Wall -Wextra -pedantic -Werror -O3)
target_include_directories(textmode PUBLIC include)

    target_compile_definitions(textmode PRIVATE 
            PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=360
            PICO_SCANVIDEO_SCANLINE_BUFFER_COUNT=4
            PICO_SCANVIDEO_PLANE1_FIXED_FRAGMENT_DMA=1

            PICO_DEFAULT_UART=1
            PICO_DEFAULT_UART_TX_PIN=20
            PICO_DEFAULT_UART_RX_PIN=21
    )

    target_link_libraries(textmode PRIVATE
            pico_multicore
            pico_stdlib
            pico_scanvideo_dpi
            hardware_dma
        )
    
    pico_add_extra_outputs(textmode)
