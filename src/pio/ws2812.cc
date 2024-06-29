#include <hardware/clocks.h>

#include "ws2812.hh"

void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin) {
  pio_gpio_init(pio, pin);

  auto div = clock_get_hz(clk_sys) / (800'000.f * ws2812_P);

  auto c = ws2812_program_get_default_config(offset);

  sm_config_set_clkdiv(&c, div);
  sm_config_set_sideset_pins(&c, pin);
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  sm_config_set_out_shift(&c, false, true, 32);

  const auto mask = 1 << pin;

  pio_sm_init(pio, sm, offset, &c);
  pio_sm_set_pindirs_with_mask(pio, sm, mask, mask);
  pio_sm_set_enabled(pio, sm, true);
}

void ws2812_write(PIO pio, uint32_t sm, color_t color) {
  auto [r, g, b] = color;

  uint32_t p[] = {g, r, b};

  uint32_t v =
    (p[0] << 24u) |
    (p[1] << 16u) |
    (p[2] << 8);

  pio_sm_put_blocking(pio, sm, v);
}
