/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <mutex>
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include <pico/mutex.h>
#include "hardware/spi.h"

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include "quadrature_encoder.h"
#include "quadrature_encoder.pio.h"
#include "core1_spi.h"

PIO encoders_pio = pio0;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

//
// ---- quadrature encoder interface example
//
// the PIO program reads phase A/B of a quadrature encoder and increments or
// decrements an internal counter to keep the current absolute step count
// updated. At any point, the main code can query the current count by using
// the quadrature_encoder_*_count functions. The counter is kept in a full
// 32 bit register that just wraps around. Two's complement arithmetic means
// that it can be interpreted as a 32-bit signed or unsigned value, and it will
// work anyway.
//
// As an example, a two wheel robot being controlled at 100Hz, can use two
// state machines to read the two encoders and in the main control loop it can
// simply ask for the current encoder counts to get the absolute step count. It
// can also subtract the values from the last sample to check how many steps
// each wheel as done since the last sample period.
//
// One advantage of this approach is that it requires zero CPU time to keep the
// encoder count updated and because of that it supports very high step rates.
//

quadrature_encoder x(encoders_pio, 0, 10);      // Base pin to connect the A phase of the encoder.                                                    
quadrature_encoder y(encoders_pio, 1, 12);      // The B phase must be connected to the next pin
quadrature_encoder z(encoders_pio, 2, 14);                                                 

int main() {
    stdio_init_all();

    #if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
    #warning spi/spi_slave example requires a board with SPI pins
        puts("Default SPI pins were not defined");
    #else

    //Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(spi_default, 1000 * 1000);

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
    // Make the SPI pins available to picotool
    //bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));

    // THIS LINE IS ABSOLUTELY KEY. Enables multi-byte transfers with one CS assert
    // Page 537 of the RP2040 Datasheet.
    spi_set_format(spi_default, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST); 
    spi_set_slave(spi_default, true);    

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // we don't really need to keep the offset, as this program must be loaded
    // at offset 0
    pio_add_program(encoders_pio, &quadrature_encoder_program);

    x.init();   // this calls quadrature_encoder_program_init that MUST be called after pio_add_program;
    y.init();   // otherwise will work only after picotool flash or reboot, but not on cold restart // LESSON LEARNED //
    z.init();

    multicore_launch_core1(core1_entry);

    while (1) {    
        // printf("x: %d", x.read_from_PIO());
        // printf("y: %d", y.read_from_PIO());
        // printf("z: %d", z.read_from_PIO());
        
        x.read_from_PIO();        
        y.read_from_PIO();
        z.read_from_PIO();
    }
        
    #endif
}


