#pragma once

#include <cstdint>

#include "util/map.hh"

struct config_mapping_t {
  // used item count
  uint8_t count;

  // must be motonic sequence in order to search properly
  map_item_t<int32_t, uint8_t> items[50];
};

struct config_t {
  // MIDI config
  uint8_t channel_num;
  uint8_t controller_num;

  // mapping config
  config_mapping_t mapping;
};

struct config_page_t {
  uint8_t pad[2];

  uint8_t free: 1;
  config_t config;
};

struct config_block_t {
  config_page_t page[16];
};

extern config_page_t config_shadow;

static auto &config = config_shadow.config;

static auto &mapping = config.mapping;

void config_save();
void config_load();
