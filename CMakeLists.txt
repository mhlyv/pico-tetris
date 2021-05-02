cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(tetris CXX C ASM)
pico_sdk_init()

file(GLOB src_main
	*.c
	*.h
)

add_executable(tetris ${src_main})
target_link_libraries(tetris pico_stdlib hardware_uart hardware_i2c)
pico_add_extra_outputs(tetris)

pico_enable_stdio_uart(tetris 1)
pico_enable_stdio_usb(tetris 0)
