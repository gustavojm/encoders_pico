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

typedef int (quadrature_encoder::*ReadFunctionPtr)();
typedef int (quadrature_encoder::*WriteFunctionPtr)(int);

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
        return current_value;
    }

    PIO pio;
    int sm;
    uint PIN_AB;
    int delta = 0;
    int current_value = 0; 
    int old_value = 0;
};


struct cmd_entry {
    uint8_t cmd;
    ReadFunctionPtr read_fn;
    WriteFunctionPtr write_fn;
}


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

    cmd_entry cmd_table[3] = {
        {0x61, quadrature_encoder::read, nullptr},
        {0x62, quadrature_encoder::read, nullptr},
        {0x63, quadrature_encoder::read, nullptr},
    };

    #define MAX_MSG_LEN     (1 + (4 * 4))           // 1 command + 4 values of 4 bytes
    uint8_t in_buf[MAX_MSG_LEN] = {0};
    //uint8_t out_buf[MAX_MSG_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8_t *out_buf_ptr = nullptr;
    int current_value = 0;
    uint8_t cmd = 0;
    
    while (1) {
        if (cmd == 0) {
            spi_read_blocking(spi_default, 0xFF, in_buf, 1);
            cmd = in_buf[0];
        }
        
        //printf("Comando: %x \n", static_cast<int>(cmd));
        switch (cmd) {
            case 0x61:
                current_value = x.read();
                break;

            case 0x62:
                current_value = y.read();
                break;

            case 0x63:
                current_value = z.read();
                break;

            default:
                cmd = 0;
                current_value = 0;
                break;
        }

        //printf("%d", current_value);
        uint8_t buf[4];
        buf[0] = static_cast<uint8_t>((current_value >> 24) & 0xFF);
        buf[1] = static_cast<uint8_t>((current_value >> 16) & 0xFF);
        buf[2] = static_cast<uint8_t>((current_value >> 8) & 0xFF);
        buf[3] = static_cast<uint8_t>((current_value >> 0) & 0xFF);
        spi_write_read_blocking(spi_default, buf, in_buf, 4);
        for (int i=0; i<4 ; i++) {
            if (in_buf[i]==0x61 || in_buf[i]==0x62 || in_buf[i]==0x63) {
                cmd = in_buf[i];
                break;
            }
        }

    }
    #endif
}

    