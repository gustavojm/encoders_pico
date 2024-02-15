#ifndef QUADRATURE_ENCODER_H
#define QUADRATURE_ENCODER_H

#include "quadrature_encoder.pio.h"

extern const uint ON_BOARD_LED_PIN;

class quadrature_encoder {
public:
    quadrature_encoder(PIO pio, int sm, uint PIN_AB) : pio(pio), sm(sm), PIN_AB(PIN_AB) {
        mutex_init(&mtx);   
    }

    int read_from_PIO() {
        int val = quadrature_encoder_get_count(pio, sm);

        mutex_enter_blocking(&mtx);
        current_value = val;
        delta = current_value - old_value;
        old_value = current_value;
        if (current_value > target) {
            //gpio_put(ON_BOARD_LED_PIN, 1);
        } else {
            //gpio_put(ON_BOARD_LED_PIN, 0);
        }
        mutex_exit(&mtx);	
        return val;        
    }

    int get_count() {
        mutex_enter_blocking(&mtx);
        int val = current_value;
        mutex_exit(&mtx);
        return val;
    }

    void set_count(int val) {
        mutex_enter_blocking(&mtx);
        current_value = val;
        mutex_exit(&mtx);        
    }

    void set_target(int val) {
        target = val;
    }

    int get_target() {
        return target;
    }

    void init() {
        quadrature_encoder_program_init(pio, sm, PIN_AB, 0);
    }

    mutex_t	mtx;
    PIO pio;
    int sm;
    uint PIN_AB;
    int delta = 0;
    int current_value = 0; 
    int target = 125;
    int old_value = 0;

    static constexpr uint8_t CLEAR_COUNTERS = 0x20;
    static constexpr uint8_t CLEAR_COUNTER_X = CLEAR_COUNTERS + 1;
    static constexpr uint8_t CLEAR_COUNTER_Y = CLEAR_COUNTERS + 2;
    static constexpr uint8_t CLEAR_COUNTER_Z = CLEAR_COUNTERS + 3;
    static constexpr uint8_t CLEAR_COUNTER_W = CLEAR_COUNTERS + 4;

    static constexpr uint8_t COUNTERS = 0x30;
    static constexpr uint8_t COUNTER_X = COUNTERS + 1;
    static constexpr uint8_t COUNTER_Y = COUNTERS + 2;
    static constexpr uint8_t COUNTER_Z = COUNTERS + 3;
    static constexpr uint8_t COUNTER_W = COUNTERS + 4;

    static constexpr uint8_t TARGETS = 0x40;
    static constexpr uint8_t TARGET_X = TARGETS + 1;
    static constexpr uint8_t TARGET_Y = TARGETS + 2;
    static constexpr uint8_t TARGET_Z = TARGETS + 3;
    static constexpr uint8_t TARGET_W = TARGETS + 4;

    static constexpr uint8_t WRITE_MASK = 1<<7;
    static constexpr uint8_t CMD_MASK = 0x7F;
};

#endif      // QUADRATURE_ENCODER_H