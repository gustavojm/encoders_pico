#ifndef CORE1_SPI_H
#define CORE1_SPI_H

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

void core1_entry();

extern const uint IRQ_TO_REMA;
extern const uint SPI_ERROR_LED;

#endif // CORE1_SPI_H