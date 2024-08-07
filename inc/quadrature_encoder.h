#ifndef QUADRATURE_ENCODER_H
#define QUADRATURE_ENCODER_H

#include "quadrature_encoder_constants.h"
#include "quadrature_encoder.pio.h"

extern const uint ON_BOARD_LED_PIN;
extern const uint IRQ_TO_REMA;
extern uint8_t targets_reached;

class quadrature_encoder {
public:
    quadrature_encoder(PIO pio, int sm, uint PIN_AB, uint PIN_DIR) : pio(pio), sm(sm), PIN_AB(PIN_AB), PIN_DIR(PIN_DIR) {
        if (PIN_DIR > 0 && PIN_DIR < 29) {
            gpio_init(PIN_DIR);
            gpio_set_dir(PIN_DIR, GPIO_OUT);
        }
        mutex_init(&mtx);   
    }

    int read_from_PIO() {
        mutex_enter_blocking(&mtx);
        int val = quadrature_encoder_get_count(pio, sm);
        current_value = val + offset;
        int error = target - current_value;
        bool already_there = (std::abs(error) < pos_threshold);        
        uint8_t targets_new = already_there ? (targets_reached | 1 << sm) : (targets_reached & ~(1 << sm));
        if (targets_new > targets_reached) {
            gpio_put(IRQ_TO_REMA, 1);          
        }                        
        targets_reached = targets_new;
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
        offset += (val - current_value);
        //printf("sc: %d \n", offset);
        mutex_exit(&mtx);
    }

    void set_target(int val) {
        mutex_enter_blocking(&mtx);   
        target = val;
        // if (sm==1) {
        //     printf("TARGET: %d\n", target);
        //     printf("c: %d\n", current_value);
        // }
        mutex_exit(&mtx);
    }

    int get_target() {
        return target;
    }

    int get_pos_threshold() {
        return pos_threshold;
    }

    void set_pos_threshold(int val) {
        pos_threshold = val;
        //printf("%d \n", pos_threshold);
    }
    
    void set_direction(bool dir) {
        if (PIN_DIR > 0 && PIN_DIR < 29) {
            gpio_put(PIN_DIR, dir);
        }
        //printf("%d \n", pos_threshold);
    }

    void init() {
        quadrature_encoder_program_init(pio, sm, PIN_AB, 0);
    }

    mutex_t	mtx;
    PIO pio;
    int sm;
    uint PIN_AB;
    uint PIN_DIR;
    volatile int current_value = 0; 
    volatile int offset;
    volatile int target = 0;
    volatile int pos_threshold = 1;
};

#endif      // QUADRATURE_ENCODER_H