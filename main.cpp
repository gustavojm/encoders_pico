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
const uint ON_BOARD_LED_PIN = PICO_DEFAULT_LED_PIN;

const uint IRQ_TO_REMA = 20;
const uint SPI_ERROR_LED = 21;
const uint SERVO_PIN = 22;

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

quadrature_encoder x(encoders_pio, 0, 8, 26);      // Base pin to connect the A phase of the encoder.                                                    
quadrature_encoder y(encoders_pio, 1, 10, 27);     // The B phase must be connected to the next pin
quadrature_encoder z(encoders_pio, 2, 12, 28);                                                 
quadrature_encoder w(encoders_pio, 3, 14, 255);                                                 

quadrature_encoder *axes_tbl[8] = {nullptr, &x, &y, &z, &w, nullptr, nullptr, nullptr};

pwm_servo servo(SERVO_PIN);

void gpio_callback(uint gpio, uint32_t events) {
    gpio_put(IRQ_TO_REMA, 1);
}

int unused_gpios[] = {6, 7};

int main() {
    busy_wait_ms(400);          // Wait for initial clock stabilization
    
    stdio_init_all();

    for (auto gpio : unused_gpios) {    // Avoid floating inputs
        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_down(gpio);
    }
    
    gpio_init(ON_BOARD_LED_PIN);
    gpio_set_dir(ON_BOARD_LED_PIN, GPIO_OUT);

    gpio_init(IRQ_TO_REMA);
    gpio_set_dir(IRQ_TO_REMA, GPIO_OUT);

    gpio_init(SPI_ERROR_LED);
    gpio_set_dir(SPI_ERROR_LED, GPIO_OUT);


    for (int hard_limit_input_pin = 0; hard_limit_input_pin < 7; hard_limit_input_pin++) {
        gpio_init(hard_limit_input_pin);
        #ifdef WITHOUT_PONCHO
           gpio_pull_up(hard_limit_input_pin);
        #else
           gpio_disable_pulls(hard_limit_input_pin);
        #endif
        gpio_set_irq_enabled_with_callback(hard_limit_input_pin, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    }    

    // we don't really need to keep the offset, as this program must be loaded
    // at offset 0
    pio_add_program(encoders_pio, &quadrature_encoder_program);

    x.init();   // this calls quadrature_encoder_program_init that MUST be called after pio_add_program;
    y.init();   // otherwise will work only after picotool flash or reboot, but not on cold restart ** LESSON LEARNED **
    z.init();
    w.init();

    multicore_launch_core1(core1_entry);
      
    while (1) {    
        x.read_from_PIO();        
        y.read_from_PIO();
        z.read_from_PIO();
        w.read_from_PIO();
    }
        
}


