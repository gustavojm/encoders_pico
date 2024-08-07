cmake_minimum_required(VERSION 3.25)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(quadrature_encoders C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_executable(${PROJECT_NAME})

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)

target_sources(${PROJECT_NAME} PRIVATE main.cpp core1_spi.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
  inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        pico_multicore
        hardware_pio
        hardware_spi
        )

target_compile_definitions(${PROJECT_NAME}
        PUBLIC PICO_DEFAULT_SPI_RX_PIN=16
        PUBLIC PICO_DEFAULT_SPI_SCK_PIN=18
        PUBLIC PICO_DEFAULT_SPI_TX_PIN=19
        PUBLIC PICO_DEFAULT_SPI_CSN_PIN=17
        PUBLIC WITHOUT_PONCHO
        )
  

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

