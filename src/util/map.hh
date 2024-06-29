#pragma once

#include <cstdint>
#include <tuple>

using std::tuple;

template <typename In, typename Out>
struct [[gnu::packed]] map_item_t {
  In in;
  Out out;
};

enum map_res_t : int8_t {
  MAP_UNDERFLOW = -1,
  MAP_OK,
  MAP_OVERFLOW,
};

// look up data in map
template <typename In, typename Out, typename Size>
static tuple<map_res_t, Out> map(const volatile map_item_t<In, Out> map[], Size size, In in) {
  const auto
    first = &map[0],
    last = &map[size - 1];

  if (
    in < first->in ||
    in == first->in
  ) return {MAP_UNDERFLOW, first->out};

  if (in > last->in) return {MAP_OVERFLOW, last->out};

  auto
    start = first,
    stop = last;

  while (start < stop) {
    auto mid = start + (stop - start) / 2;

    if (mid->in < in)
      start = mid + 1;
    else
      stop = mid;
  }

  auto pre = start - 1;

  // interpolate value
  Out out = pre->out + (float) (in - pre->in) / (start->in - pre->in) * (start->out - pre->out);

  return {MAP_OK, out};
}
