#pragma once

#include <tuple>

// interpolate `val` to following color continuously: blue, cyan, green, yellow, red, magenta
template <typename Data, Data val_min, Data val_max, uint8_t bright_min = 0, uint8_t bright_max = 16>
color_t colorful(Data val) {
  float scale = 5.f * (val - val_min) / (val_max - val_min);

  uint8_t whole = scale;

  float exp = (scale - whole) * (bright_max - bright_min);

  uint8_t inc = bright_min + exp;
  uint8_t dec = bright_max - exp;

  switch (whole) {
  case 0: return {bright_min, inc, bright_max}; // (0, 0, 1): increasing green
  case 1: return {bright_min, bright_max, dec}; // (0, 1, 1): decreasing blue
  case 2: return {inc, bright_max, bright_min}; // (0, 1, 0): increasing red
  case 3: return {bright_max, dec, bright_min}; // (1, 1, 0): decreasing green
  case 4: return {bright_max, bright_min, inc}; // (1, 0, 0): increasing blue
  case 5: return {bright_max, bright_min, bright_max}; // (1, 0, 1)
  }

  return {0, 0, 0};
}
