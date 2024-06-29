#pragma once

#include <hardware/pio.h>

#include "tm7711.pio.h"

enum tm7711_read_mode : uint8_t {
  TM7711_READ_FAST = 2,
};

void tm7711_program_init(PIO pio, uint32_t sm, uint32_t offset, uint32_t in_pin, uint32_t clk_pin);

static inline void tm7711_read_start(PIO pio, uint32_t sm, tm7711_read_mode mode) {
  pio_sm_put_blocking(pio, sm, mode);
}

static inline bool tm7711_read_available(PIO pio, uint32_t sm) {
  return !pio_sm_is_rx_fifo_empty(pio, sm);
}

int32_t tm7711_read(PIO pio, uint32_t sm);
