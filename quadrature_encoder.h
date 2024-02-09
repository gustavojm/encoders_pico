#ifndef QUADRATURE_ENCODER_H
#define QUADRATURE_ENCODER_H

#include "quadrature_encoder.pio.h"

#define ENCODER_CLEAR_COUNTERS     0x20
#define ENCODER_CLEAR_COUNTER_X    ENCODER_CLEAR_COUNTERS + 1
#define ENCODER_CLEAR_COUNTER_Y    ENCODER_CLEAR_COUNTERS + 2
#define ENCODER_CLEAR_COUNTER_Z    ENCODER_CLEAR_COUNTERS + 3
#define ENCODER_CLEAR_COUNTER_W    ENCODER_CLEAR_COUNTERS + 4

#define ENCODER_COUNTERS           0x60
#define ENCODER_COUNTER_X          ENCODER_COUNTERS + 1
#define ENCODER_COUNTER_Y          ENCODER_COUNTERS + 2
#define ENCODER_COUNTER_Z          ENCODER_COUNTERS + 3
#define ENCODER_COUNTER_W          ENCODER_COUNTERS + 4

#define ENCODER_TARGETS            0xE0
#define ENCODER_TARGET_X           ENCODER_TARGETS + 1
#define ENCODER_TARGET_Y           ENCODER_TARGETS + 2
#define ENCODER_TARGET_Z           ENCODER_TARGETS + 3
#define ENCODER_TARGET_W           ENCODER_TARGETS + 4

#define MAX_MSG_LEN     (1 + (4 * 4))           // 1 command + 4 values of 4 bytes

extern const uint LED_PIN;

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
            gpio_put(LED_PIN, 1);
        } else {
            gpio_put(LED_PIN, 0);
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
};

#endif      // QUADRATURE_ENCODER_H