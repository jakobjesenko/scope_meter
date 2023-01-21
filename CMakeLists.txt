cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(scope_meter_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
pico_sdk_init()

add_executable(scope_meter
    scope_meter.c
)

pico_generate_pio_header(scope_meter ${CMAKE_CURRENT_LIST_DIR}/measure.pio)
pico_generate_pio_header(scope_meter ${CMAKE_CURRENT_LIST_DIR}/pulse.pio)

target_sources(scope_meter PRIVATE scope_meter.c)

pico_enable_stdio_usb(scope_meter 1)
pico_enable_stdio_uart(scope_meter 0)


target_link_libraries(scope_meter
    pico_stdlib
    hardware_pio
    hardware_gpio
    hardware_adc
    hardware_dma
    pico_multicore
)

pico_add_extra_outputs(scope_meter)