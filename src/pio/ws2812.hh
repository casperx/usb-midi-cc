#pragma once

#include <tuple>

#include <hardware/pio.h>

#include "ws2812.pio.h"

using color_t = std::tuple<uint8_t, uint8_t, uint8_t>;

void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin);

void ws2812_write(PIO pio, uint32_t sm, color_t color);
