#pragma once

#include <cstdint>
#include <hardware/spi.h>
#include <mutex>
#include <pico/binary_info.h>
#include <pico/multicore.h>
#include <pico/mutex.h>
#include <stdio.h>
#include <string.h>

#include <hardware/pio.h>
#include <hardware/timer.h>
#include <pico/stdlib.h>

#include "quadrature_encoder.h"
#include "pwm_servo.h"

void core1_entry();

extern const uint IRQ_TO_REMA;
extern const uint SPI_ERROR_LED;
extern pwm_servo servo;
