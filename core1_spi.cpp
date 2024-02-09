#include "core1_spi.h"

extern quadrature_encoder x, y, z;

int spi_tx_rx(int current_value){
    // printf("cv: %d", current_value);
    uint8_t in_buf[4];
    uint8_t buf[4];
    buf[0] = static_cast<uint8_t>((current_value >> 24) & 0xFF);
    buf[1] = static_cast<uint8_t>((current_value >> 16) & 0xFF);
    buf[2] = static_cast<uint8_t>((current_value >> 8) & 0xFF);
    buf[3] = static_cast<uint8_t>((current_value >> 0) & 0xFF);
    spi_write_read_blocking(spi_default, buf, in_buf, 4);
    return static_cast<int>(in_buf[0] << 24 | in_buf[1] << 16 | in_buf[2] << 8 | in_buf[3] << 0);
}

void core1_entry() {
    while (1) {        
        uint8_t in_buf[MAX_MSG_LEN] = {0};
        //uint8_t out_buf[MAX_MSG_LEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
        uint8_t *out_buf_ptr = nullptr;

        // int prev = z.get_count();
        // printf("prev %d", prev);

        spi_read_blocking(spi_default, 0xEA, in_buf, 1);
        uint8_t cmd = in_buf[0];
        //printf("Cmd: %x \n", static_cast<int>(cmd));
        
        int current_value = 0;
        int val = 0;
        switch (cmd & 0x7F)
        {
        case ENCODER_COUNTER_X:
            current_value = x.get_count();
            val = spi_tx_rx(current_value);
            if (cmd & 0x80) {
                x.set_count(val);
            }
            break;
        
        case ENCODER_COUNTER_Y:
            current_value = y.get_count();
            val = spi_tx_rx(current_value);
            if (cmd & 0x80) {
                x.set_count(val);
            }
            break;

        case ENCODER_COUNTER_Z:
            current_value = z.get_count();
            val = spi_tx_rx(current_value);
            if (cmd & 0x80) {
                x.set_count(val);
            }
            break;

        default:
            break;
        }
        
    }

}