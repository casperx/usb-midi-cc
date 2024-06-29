#pragma once

#include <cstdint>

struct [[gnu::packed]] quota_group_t {
  // each bit corresponds to each slot
  uint8_t free;
  uint16_t slot[8];
};

struct quota_page_t {
  uint8_t pad[1];
  quota_group_t group[15];
};

struct quota_block_t {
  quota_page_t page[16];
};

extern uint16_t quota_remain;

void quota_save();
void quota_load();
