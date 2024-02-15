#include "core1_spi.h"

#define HARD_LIMITS     0x40

extern quadrature_encoder x, y, z;

void fill_buf(uint8_t * buf, int value) {
    buf[0] = static_cast<uint8_t>((value >> 24) & 0xFF);
    buf[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
    buf[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
    buf[3] = static_cast<uint8_t>((value >> 0) & 0xFF);
}

int spi_tx_rx(int current_value){
    // printf("cv: %d", current_value);
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
        uint8_t in_buf[1] = {0};
        uint8_t out_buf[12];

        // int prev = z.get_count();
        // printf("prev %d", prev);

        spi_read_blocking(spi_default, 0x00, in_buf, 1);
        uint8_t cmd = in_buf[0];        
        //printf("Cmd: %x \n", static_cast<int>(cmd));
        
        int current_value;
        uint8_t limits;
        int val;

        switch (cmd & quadrature_encoder::CMD_MASK) {
            case quadrature_encoder::COUNTERS:            
                gpio_put(ON_BOARD_LED_PIN, 0);
                fill_buf(out_buf, x.get_count());
                fill_buf(&(out_buf[4]), y.get_count());
                fill_buf(&(out_buf[8]), z.get_count());

                spi_write_blocking(spi_default, out_buf, 12);
                break;

            case quadrature_encoder::COUNTER_X:
                current_value = x.get_count();
                val = spi_tx_rx(current_value);
                if (cmd & quadrature_encoder::WRITE_MASK) {
                    x.set_count(val);
                }
                break;
            
            case quadrature_encoder::COUNTER_Y:
                current_value = y.get_count();
                val = spi_tx_rx(current_value);
                if (cmd & quadrature_encoder::WRITE_MASK) {
                    x.set_count(val);
                }
                break;

            case quadrature_encoder::COUNTER_Z:
                gpio_put(ON_BOARD_LED_PIN, 0);
                current_value = z.get_count();
                val = spi_tx_rx(current_value);
                if (cmd & quadrature_encoder::WRITE_MASK) {
                    x.set_count(val);
                }
                break;

            case quadrature_encoder::CLEAR_COUNTERS:
                x.set_count(0);
                y.set_count(0);
                z.set_count(0);
                break;

            case quadrature_encoder::CLEAR_COUNTER_X:
                x.set_count(0);
                break;

            case quadrature_encoder::CLEAR_COUNTER_Y:
                y.set_count(0);
                break;

            case quadrature_encoder::CLEAR_COUNTER_Z:
                z.set_count(0);
                break;

            case quadrature_encoder::TARGET_X:
                current_value = x.get_target();
                val = spi_tx_rx(current_value);
                if (cmd & quadrature_encoder::WRITE_MASK) {
                    x.set_target(val);
                }
                break;

            case quadrature_encoder::TARGET_Y:
                current_value = y.get_target();
                val = spi_tx_rx(current_value);
                if (cmd & quadrature_encoder::WRITE_MASK) {
                    y.set_target(val);
                }
                break;

            case quadrature_encoder::TARGET_Z:
                current_value = z.get_target();
                val = spi_tx_rx(current_value);
                if (cmd & quadrature_encoder::WRITE_MASK) {
                    z.set_target(val);
                }
                break;

            case HARD_LIMITS:
                if (cmd & quadrature_encoder::WRITE_MASK) {
                } else {
                    limits = gpio_get_all() & 0xFF;
                    spi_write_blocking(spi_default, &limits, 1);
                }
                break;

            default:               
                gpio_put(SPI_ERROR_LED, 1);
                sleep_us(10);       // very important... helps to resync in case of restart of raspberry pi pico 
                break;
        }
        
    }
    
    #endif
}