#pragma once

#include <tuple>

#include <hardware/pio.h>

#include "ws2812.pio.h"

using std::tuple;

void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin);

static constexpr uint32_t ws2812_color(
  uint8_t r,
  uint8_t g,
  uint8_t b
) {
  uint32_t p[] = {g, r, b};

  return
    (p[0] << 24u) |
    (p[1] << 16u) |
    (p[2] << 8);
}

static constexpr uint32_t ws2812_color(
  tuple<uint8_t, uint8_t, uint8_t> color
) {
  auto [r, g, b] = color;

  return ws2812_color(r, g, b);
}
