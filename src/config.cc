#include <string.h>

#include <pico.h>

#include <hardware/sync.h>
#include <hardware/flash.h>

#include "config.hh"

static constexpr config_block_t config_init() {
  config_block_t block = {};

  auto &page = block.page[count_of(block.page) - 1];

  auto &config = page.config;

  config.channel_num = 0;
  config.controller_num = 2;

  auto &map = config.mapping;

  map.count = 2;

  auto &entry0 = map.items[0];
  auto &entry1 = map.items[1];

  entry0.in = 1'400'000;
  entry0.out = 0;

  entry1.in = 3'500'000;
  entry1.out= 127;

  return block;
}

static volatile config_block_t config_block [[
  gnu::section(".flash_persist")
]] = (
  config_init()
);

static_assert(sizeof(config_block) % 4'096 == 0);

config_page_t config_shadow;

static_assert(sizeof(config_shadow) % 256 == 0);

static uint8_t config_page [[
  gnu::section(".uninitialized_data")
]];

void config_save() {
  auto ints = save_and_disable_interrupts();

  if (config_page == count_of(config_block.page)) {
    // block is full
    flash_range_erase((uint32_t) &config_block - XIP_BASE, sizeof(config_block));

    config_page = 0;
  }

  // persist
  flash_range_program((uint32_t) &config_block.page[config_page] - XIP_BASE, (uint8_t *) &config_shadow, sizeof(config_shadow));

  ++config_page;

  restore_interrupts(ints);
}

void config_load() {
  config_page = count_of(config_block.page);

  // find last programmed config
  while (config_page > 0) {
    --config_page;

    const auto &page = config_block.page[config_page];

    if (page.free == 0) {
      // load shadow
      memcpy((uint8_t *) &config_shadow, (uint8_t *) &page, sizeof(config_shadow));

      // go to next free slot
      ++config_page;

      return;
    }
  }
}
