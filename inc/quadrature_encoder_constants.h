#ifndef QUADRATURE_ENCODER_CONSTANTS_H
#define QUADRATURE_ENCODER_CONSTANTS_H

#include <cstdint>

namespace quadrature_encoder_constants {
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

    static constexpr uint8_t POS_THRESHOLDS = 0x50;
    static constexpr uint8_t POS_THRESHOLD_X = POS_THRESHOLDS + 1;
    static constexpr uint8_t POS_THRESHOLD_Y = POS_THRESHOLDS + 2;
    static constexpr uint8_t POS_THRESHOLD_Z = POS_THRESHOLDS + 3;
    static constexpr uint8_t POS_THRESHOLD_W = POS_THRESHOLDS + 4;

    static constexpr uint8_t WRITE_MASK = 1<<7;
    static constexpr uint8_t CMD_MASK = 0x70;

    static constexpr uint8_t LIMITS = 0x60;
}

#endif      // QUADRATURE_ENCODER_CONSTANTS_H