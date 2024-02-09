cmake_minimum_required(VERSION 3.25)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(encoders C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_executable(quadrature_encoder_cpp)

pico_generate_pio_header(quadrature_encoder_cpp ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)

target_sources(quadrature_encoder_cpp PRIVATE main.cpp core1_spi.cpp)

target_link_libraries(quadrature_encoder_cpp PRIVATE
        pico_stdlib
        pico_multicore
        hardware_pio
        hardware_spi
        )

target_compile_definitions(quadrature_encoder_cpp 
        PUBLIC PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
        PUBLIC PICO_DEFAULT_SPI_RX_PIN=16
        PUBLIC PICO_DEFAULT_SPI_SCK_PIN=18
        PUBLIC PICO_DEFAULT_SPI_TX_PIN=19
        PUBLIC PICO_DEFAULT_SPI_CSN_PIN=17
        )
  

# enable usb output, disable uart output
pico_enable_stdio_usb(quadrature_encoder_cpp 1)
pico_enable_stdio_uart(quadrature_encoder_cpp 0)

pico_add_extra_outputs(quadrature_encoder_cpp)

