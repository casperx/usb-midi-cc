#include "tm7711.hh"

void tm7711_program_init(PIO pio, uint32_t sm, uint32_t offset, uint32_t in_pin, uint32_t clk_pin) {
  pio_gpio_init(pio, in_pin);
  pio_gpio_init(pio, clk_pin);

  auto c = tm7711_program_get_default_config(offset);

  sm_config_set_clkdiv(&c, 500.f);

  sm_config_set_in_pins(&c, in_pin);
  sm_config_set_sideset_pins(&c, clk_pin);

  sm_config_set_in_shift(&c, false, true, 24);

  const auto in_mask = 1 << in_pin, clk_mask = 1 << clk_pin;

  hw_set_bits(&pio->input_sync_bypass, in_mask);

  pio_sm_init(pio, sm, offset, &c);

  pio_sm_set_pindirs_with_mask(pio, sm, clk_mask, in_mask | clk_mask);
  pio_sm_set_pins_with_mask(pio, sm, 0, in_mask | clk_mask);

  pio_sm_set_enabled(pio, sm, true);
}


int32_t tm7711_read(PIO pio, uint32_t sm) {
  auto raw_val = pio_sm_get(pio, sm);

  // sign extended 24 bits value
  return
    raw_val & 0x800000 ?
      raw_val | 0xFF000000 :
      raw_val;
}
