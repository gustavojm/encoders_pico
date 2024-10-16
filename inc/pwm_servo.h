#pragma once

#include "pico/stdlib.h"
#include "hardware/pwm.h"


class pwm_servo {
public:
    pwm_servo(uint PIN) : PWM_PIN(PIN) {
        // Initialize the chosen GPIO pin for PWM
        gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);

        // Find the PWM slice for the selected GPIO pin
        slice_num = pwm_gpio_to_slice_num(PWM_PIN);

        // Configure PWM frequency
        pwm_set_clkdiv(slice_num, 125); // PWM clock should now be running at 1MHz
        pwm_set_wrap(slice_num, 3000 - 1);  // Set period of 3000 cycles (0 to 2999 inclusive)
        pwm_set_output_polarity(slice_num, true, false);
    }

    // Function to set servo angle
    void set_angle(float angle) {
        // Calculate the duty cycle for the desired angle (assuming 30ms period PWM)
        // Servo expects pulses between 0.5ms (0 degrees) and 2.5ms (180 degrees)
        int min_pulse_width = 500;   // Minimum pulse width in microseconds
        int max_pulse_width = 2500;  // Maximum pulse width in microseconds
        float pulse_width = min_pulse_width + (angle / 180.0f) * (max_pulse_width - min_pulse_width);

        // Enable PWM
        pwm_set_enabled(slice_num, true);

        pwm_set_gpio_level(PWM_PIN, static_cast<int>(pulse_width));
        sleep_ms(20);  // 2 Cycles are enough for it to latch the desired position, sending at least 3
        
        pwm_set_enabled(slice_num, false);
    }
    
    uint PWM_PIN;
    uint slice_num;
};