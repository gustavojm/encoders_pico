/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include "quadrature_encoder.pio.h"

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
        return current_value;
    }

    PIO pio;
    int sm;
    uint PIN_AB;
    int delta = 0;
    int current_value = 0; 
    int old_value = 0;
};

quadrature_encoder& x() {
    static quadrature_encoder qe(encoders_pio, 0, 10);    // Base pin to connect the A phase of the encoder.
    return qe;                                            // The B phase must be connected to the next pin
}

quadrature_encoder& y() {
    static quadrature_encoder qe(encoders_pio, 1, 12);    // Base pin to connect the A phase of the encoder.
    return qe;                                            // The B phase must be connected to the next pin
}

quadrature_encoder& z() {
    static quadrature_encoder qe(encoders_pio, 2, 14);    // Base pin to connect the A phase of the encoder.
    return qe;                                            // The B phase must be connected to the next pin
}


int main() {
    stdio_init_all();

    // we don't really need to keep the offset, as this program must be loaded
    // at offset 0
    pio_add_program(encoders_pio, &quadrature_encoder_program);


    while (1) {
        // note: thanks to two's complement arithmetic delta will always
        // be correct even when new_value wraps around MAXINT / MININT
        x().read();
        y().read();
        z().read();

        printf("X position %8d, delta %6d | Y position %8d, delta %6d | Z position %8d, delta %6d |\n", 
                x().current_value, x().delta, 
                y().current_value, y().delta, 
                z().current_value, z().delta);
        sleep_ms(100);
    }
}

