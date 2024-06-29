#include <string.h>

#include <pico.h>

#include <hardware/sync.h>
#include <hardware/flash.h>

#include "quota.hh"

static constexpr quota_block_t quota_init()  {
  quota_block_t block = {};

  auto &page = block.page[count_of(block.page) - 1];
  auto &group = page.group[count_of(page.group) - 1];

  group.slot[count_of(group.slot) - 1] = 1'440; // 1 day

  return block;
}

static volatile quota_block_t quota_block [[
  gnu::section(".flash_persist")
]] = (
  quota_init()
);

static_assert(sizeof(quota_block) % 4'096 == 0);

static quota_page_t quota_shadow;

static_assert(sizeof(quota_shadow) % 256 == 0);

static uint8_t quota_page [[
  gnu::section(".uninitialized_data")
]];

static uint8_t quota_group [[
  gnu::section(".uninitialized_data")
]];

static uint8_t quota_index [[
  gnu::section(".uninitialized_data")
]];

// tick every 1 minute get us 45 days period
uint16_t quota_remain [[
  gnu::section(".uninitialized_data")
]];

void quota_save() {
  auto ints = save_and_disable_interrupts();

  if (quota_index == 8) {
    ++quota_group;
    quota_index = 0;
  }

  if (quota_group == count_of(quota_shadow.group)) {
    // reset shadow page
    memset((uint8_t *) &quota_shadow, 0xFF, sizeof(quota_shadow));

    ++quota_page;
    quota_group = 0;
  }

  if (quota_page == count_of(quota_block.page)) {
    // block is full
    flash_range_erase((uint32_t) &quota_block - XIP_BASE, sizeof(quota_block));

    quota_page = 0;
  }

  auto &group = quota_shadow.group[quota_group];

  group.free &= ~(1 << quota_index);
  group.slot[quota_index] = quota_remain;

  // persist
  flash_range_program((uint32_t) &quota_block.page[quota_page] - XIP_BASE, (uint8_t *) &quota_shadow, sizeof(quota_shadow));

  ++quota_index;

  restore_interrupts(ints);
}

void quota_load() {
  quota_page = count_of(quota_block.page);

  // find last programmed quota
  while (quota_page > 0) {
    --quota_page;
    quota_group = count_of(quota_shadow.group);

    const auto &page = quota_block.page[quota_page];

    while (quota_group > 0) {
      --quota_group;
      quota_index = 8;

      const auto &group = page.group[quota_group];

      while (quota_index > 0) {
        --quota_index;

        const auto free = group.free & (1u << quota_index);

        if (free == 0) {
          // load shadow page
          memcpy((uint8_t *) &quota_shadow, (uint8_t *) &page, sizeof(quota_shadow));

          // load data
          quota_remain = quota_shadow.group[quota_group].slot[quota_index];

          // go to next free slot
          ++quota_index;

          return;
        }
      }
    }
  }
}
