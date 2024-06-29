#pragma once

#include <hardware/pio.h>

#include "tm7711.pio.h"

void tm7711_program_init(PIO pio, uint32_t sm, uint32_t offset, uint32_t in_pin, uint32_t clk_pin);
