/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/binary_info.h"
#include "hardware/spi.h"

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include "quadrature_encoder.pio.h"
#include "encoder.h"


PIO encoders_pio = pio0;

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

class quadrature_encoder {
public:
    quadrature_encoder(PIO pio, int sm, uint PIN_AB) : pio(pio), sm(sm), PIN_AB(PIN_AB) {
        quadrature_encoder_program_init(pio, sm, PIN_AB, 0);    
    }

    int read() {
        current_value = quadrature_encoder_get_count(pio, sm);
        delta = current_value - old_value;
        old_value = current_value;
        current_value_spi_buf[0] = static_cast<uint8_t>((current_value >> 24) & 0xFF);
        current_value_spi_buf[1] = static_cast<uint8_t>((current_value >> 16) & 0xFF);
        current_value_spi_buf[2] = static_cast<uint8_t>((current_value >> 8) & 0xFF);
        current_value_spi_buf[3] = static_cast<uint8_t>((current_value >> 0) & 0xFF);
        return current_value;

    }

    PIO pio;
    int sm;
    uint PIN_AB;
    int delta = 0;
    int current_value = 0; 
    uint8_t current_value_spi_buf[4];
    int old_value = 0;
};


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

    // we don't really need to keep the offset, as this program must be loaded
    // at offset 0
    pio_add_program(encoders_pio, &quadrature_encoder_program);
    
    quadrature_encoder x(encoders_pio, 0, 10);      // Base pin to connect the A phase of the encoder.                                                    
    quadrature_encoder y(encoders_pio, 1, 12);      // The B phase must be connected to the next pin
    quadrature_encoder z(encoders_pio, 2, 14);                                                 

    #define MAX_MSG_LEN     (1 + (4 * 4))           // 1 command + 4 values of 4 bytes
    uint8_t in_buf[MAX_MSG_LEN] = {0};
    //uint8_t out_buf[MAX_MSG_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8_t *out_buf_ptr = nullptr;
    
    while (1) {      
        if (!out_buf_ptr) {
            spi_read_blocking(spi_default, 0xFF, in_buf, 1);
        }
        printf("Comando: %x \n", static_cast<int>(in_buf[0]));        
        switch (in_buf[0])
        {
        case 0x61:
            x.read();
            out_buf_ptr = x.current_value_spi_buf;
            break;

        case 0x62:
            y.read();
            out_buf_ptr = y.current_value_spi_buf;
            break;

        case 0x63:
            z.read();
            out_buf_ptr = z.current_value_spi_buf;
            break;

        default:
            out_buf_ptr = nullptr;
            break;
        }

        if (out_buf_ptr) {
            spi_write_read_blocking(spi_default, out_buf_ptr, in_buf, 4);
            printf("sending: %d\n", *out_buf_ptr);
        }

        printf("ptr addr: %p\n", out_buf_ptr);

        // spi_write_read_blocking(spi_default, out_buf, in_buf, 15);
        // for (int i = 0; i < MAX_MSG_LEN; ++i) {
        //     printf("%x :", static_cast<int>(in_buf[i]));
        // }
        // spi_read_blocking(spi_default, 0x00, in_buf, 1);
        // switch (in_buf[0]) {
        //     case ENCODER_READ_COUNTER_X:
        //         x.read();
        //         printf("res X: %d %d %d %d |", x.current_value_spi_buf[0], x.current_value_spi_buf[1], x.current_value_spi_buf[2], x.current_value_spi_buf[3]);
        //         spi_write_blocking(spi_default, x.current_value_spi_buf, 4);
        //         /* code */
        //         break;

        //     case ENCODER_READ_COUNTER_Y:
        //         y.read();
        //         printf("res Y: %d %d %d %d |", y.current_value_spi_buf[0], y.current_value_spi_buf[1], y.current_value_spi_buf[2], y.current_value_spi_buf[3]);
        //         spi_write_blocking(spi_default, y.current_value_spi_buf, 4);
        //         /* code */
        //         break;

        //     case ENCODER_READ_COUNTER_Z:
        //         z.read();
        //         printf("res Z: %d %d %d %d |\n", z.current_value_spi_buf[0], z.current_value_spi_buf[1], z.current_value_spi_buf[2], z.current_value_spi_buf[3]);
        //         spi_write_blocking(spi_default, z.current_value_spi_buf, 4);
        //         /* code */
        //         break;

        //     default:
        //         break;
        // }

        // note: thanks to two's complement arithmetic delta will always
        // be correct even when new_value wraps around MAXINT / MININT
        // x.read();
        // y.read();
        // z.read();

        // printf("X position %8d, delta %6d | Y position %8d, delta %6d | Z position %8d, delta %6d |\n", 
        //         x.current_value, x.delta, 
        //         y.current_value, y.delta, 
        //         z.current_value, z.delta);
        // sleep_ms(100);
    }    
    #endif
}

    