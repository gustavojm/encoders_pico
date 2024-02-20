#include "core1_spi.h"

extern quadrature_encoder *axes_tbl[];
uint8_t targets_reached;

void fill_buf(uint8_t * buf, int value) {
    buf[0] = static_cast<uint8_t>((value >> 24) & 0xFF);
    buf[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
    buf[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
    buf[3] = static_cast<uint8_t>((value >> 0) & 0xFF);
}

int spi_tx_rx(int current_value){
    uint8_t in_buf[4];
    uint8_t buf[4];
    fill_buf(buf, current_value);
    spi_write_read_blocking(spi_default, buf, in_buf, 4);
    return static_cast<int>(in_buf[0] << 24 | in_buf[1] << 16 | in_buf[2] << 8 | in_buf[3] << 0);
}

void core1_entry() {
    #if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
    #warning spi/spi_slave example requires a board with SPI pins
        puts("Default SPI pins were not defined");
    #else

    //Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(spi_default, 1000 * 1000);

    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
    
    // Make the SPI pins available to picotool    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnarrowing"
    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));
    #pragma GCC diagnostic pop

    // THIS LINE IS ABSOLUTELY KEY. Enables multi-byte transfers with one CS assert
    // Page 537 of the RP2040 Datasheet.
    spi_set_format(spi_default, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST); 
    spi_set_slave(spi_default, true);

    while (1) {        
        uint8_t in_buf[4] = {0};
        uint8_t out_buf[16];
       
        int current_value;
        uint8_t hard_limits;
        int val;
        quadrature_encoder *axis;

        gpio_put(SPI_ERROR_LED, 0);

        spi_read_blocking(spi_default, 0x00, in_buf, 1);
        uint8_t cmd = in_buf[0];        
        switch (cmd & quadrature_encoder_constants::CMD_MASK) {
            case quadrature_encoder_constants::COUNTERS:
                axis = axes_tbl[cmd & 0x0F];
                if (axis) {
                    current_value = axis->get_count();
                    val = spi_tx_rx(current_value);
                    if (cmd & quadrature_encoder_constants::WRITE_MASK) {
                        axis->set_count(val);
                    }
                } else {        // all axes
                    fill_buf(out_buf, axes_tbl[1]->get_count());
                    fill_buf(&(out_buf[4]), axes_tbl[2]->get_count());
                    fill_buf(&(out_buf[8]), axes_tbl[3]->get_count());
                    fill_buf(&(out_buf[12]), axes_tbl[4]->get_count());
                    spi_write_blocking(spi_default, out_buf, 16);                    
                }
                break;

            case quadrature_encoder_constants::CLEAR_COUNTERS:
                axis = axes_tbl[cmd & 0x0F];
                if (axis) {
                    axis->set_count(0);
                } else {
                    axes_tbl[1]->set_count(0);
                    axes_tbl[2]->set_count(0);
                    axes_tbl[3]->set_count(0);
                    axes_tbl[4]->set_count(0);
                }
                break;

            case quadrature_encoder_constants::TARGETS:
                axis = axes_tbl[cmd & 0x0F];
                if (axis) {
                    current_value = axis->get_target();
                    val = spi_tx_rx(current_value);
                    if (cmd & quadrature_encoder_constants::WRITE_MASK) {
                        axis->set_target(val);
                    }
                }
                break;

            case quadrature_encoder_constants::POS_THRESHOLDS:            // Same threshold for all axes
                axis = axes_tbl[cmd & 0x0F];
                if (axis) {
                    current_value = axis->get_pos_threshold();
                    val = spi_tx_rx(current_value);
                    if (cmd & quadrature_encoder_constants::WRITE_MASK) {
                        axis->set_pos_threshold(val);
                    }
                } else {
                    if (cmd & quadrature_encoder_constants::WRITE_MASK) {
                        spi_read_blocking(spi_default, 0x00, in_buf, 4);
                        int threshold = static_cast<int>(in_buf[0] << 24) |
                                        static_cast<int>(in_buf[1] << 16) |
                                        static_cast<int>(in_buf[2] << 8) |
                                        static_cast<int>(in_buf[3] << 0);

                        axes_tbl[1]->set_pos_threshold(threshold);
                        axes_tbl[2]->set_pos_threshold(threshold);
                        axes_tbl[3]->set_pos_threshold(threshold);
                        axes_tbl[4]->set_pos_threshold(threshold);
                    }
                }
                break;

            case quadrature_encoder_constants::LIMITS:
                hard_limits = gpio_get_all() & 0xFF;
                out_buf[0] = hard_limits;
                out_buf[1] = targets_reached;
                out_buf[2] = 0x00;
                out_buf[3] = 0x00;
                spi_write_blocking(spi_default, out_buf, 4);
                if (cmd & quadrature_encoder_constants::WRITE_MASK) {
                    gpio_put(IRQ_TO_REMA, 0);        // The IRQ was acknowledged
                }
                break;

            default:               
                gpio_put(SPI_ERROR_LED, 1);
                break;
        }
        
    }
    
    #endif
}