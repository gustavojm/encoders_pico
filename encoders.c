



static inline void __program_init(PIO pio, uint sm, uint offset, uint in_pin, uint in_pin_count, uint out_pin, uint out_pin_count, float frequency) {
  // 1. Define a config object
  pio_sm_config config = __program_get_default_config(offset);

  // 2. Set and initialize the input pins
  sm_config_set_in_pins(&config, in_pin);
  pio_sm_set_consecutive_pindirs(pio, sm, in_pin, in_pin_count, 1);
  pio_gpio_init(pio, in_pin);

  // 3. Set and initialize the output pins
  sm_config_set_out_pins(&config, out_pin, out_pin_count);
  pio_sm_set_consecutive_pindirs(pio, sm, out_pin, out_pin_count, 0);

  // 4. Set clock divider
  if (frequency < 2000) {
    frequency = 2000;
  }
  float clock_divider = (float) clock_get_hz(clk_sys) / frequency * 1000;
  sm_config_set_clkdiv(&config, clock_divider);

  // 5. Configure input shift register
  // args: BOOL right_shift, BOOL auto_push, 1..32 push_threshold
  sm_config_set_in_shift(&config, true, false, 32);

  // 6. Configure output shift register
  // args: BOOL right_shift, BOOL auto_push, 1..32 push_threshold
  sm_config_set_out_shift(&config, true, false, 32);

  // 7. Join the ISR & OSR
  // PIO_FIFO_JOIN_NONE = 0, PIO_FIFO_JOIN_TX = 1, PIO_FIFO_JOIN_RX = 2
  sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_NONE);

  // 8. Apply the configuration
  pio_sm_init(pio, sm, offset, &config);

  // 9. Activate the State Machine
  pio_sm_set_enabled(pio, sm, true);
}
