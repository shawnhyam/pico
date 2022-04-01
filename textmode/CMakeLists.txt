

    add_executable(textmode
            textmode.c
      )


    target_compile_definitions(textmode PRIVATE 
            PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=360
            
            PICO_DEFAULT_UART=1
            PICO_DEFAULT_UART_TX_PIN=20
            PICO_DEFAULT_UART_RX_PIN=21
    )

    target_link_libraries(textmode PRIVATE
            pico_multicore
            pico_stdlib
            pico_scanvideo_dpi
        )
    
    pico_add_extra_outputs(textmode)
