#include <pico/rand.h>

#include <hardware/pio.h>

#include <tusb.h>

#include "pio/tm7711.hh"
#include "pio/ws2812.hh"

#include "config.hh"
#include "quota.hh"
#include "emulated.hh"

#include "util/map.hh"
#include "util/colorful.hh"

// protocol definitions
namespace command {
  namespace request {
    enum : uint8_t {
      set_map = 1,
      set_channel_num,
      set_controller_num,

      save_config,
      get_config,

      dump,
    };
  }

  namespace response {
    enum : uint8_t {
      dump = 1,

      get_config,
    };
  }
}

static inline auto tud_vendor_write_char(char c) {
  return tud_vendor_write(&c, 1);
}

int main() {
  config_load();

#if defined(DEMO)
  quota_load();

  absolute_time_t next_deduct = get_absolute_time();
#endif

  tusb_init();

  gpio_set_function(25, GPIO_FUNC_SIO);
  gpio_set_dir(25, true);

#if defined(EMULATED)
  absolute_time_t
    next_out = get_absolute_time(),
    next_gen = get_absolute_time();

  int32_t gen_step = 0;
#else
  auto pio = pio0;

  auto sensor_sm = pio_claim_unused_sm(pio, true);
  auto led_sm = pio_claim_unused_sm(pio, true);

  tm7711_program_init(pio, sensor_sm, pio_add_program(pio, &tm7711_program), 29, 28);
  ws2812_program_init(pio, led_sm, pio_add_program(pio, &ws2812_program), 16);

  // first sensor read
  tm7711_read_start(pio, sensor_sm, TM7711_READ_FAST);
#endif

  bool dumped = false;

  absolute_time_t next_dumped = get_absolute_time();

  bool prev_state = true;

  for (;;) {
    tud_task();

#if defined(EMULATED)
    if (time_reached(next_gen)) {
      gen_step = (gen_step + 1) % count_of(gen_samps);

      next_gen = make_timeout_time_ms(50);
    }
#endif

#if defined(DEMO)
    if (quota_remain && time_reached(next_deduct)) {
      --quota_remain;
      quota_save();

      // every 1 minute
      next_deduct = make_timeout_time_ms(60'000);
    }
#endif

    // stop dump if no ping
    if (dumped && time_reached(next_dumped)) dumped = false;

    // handle commands
    if (tud_vendor_available()) {
      uint8_t buf;

      tud_vendor_read(&buf, sizeof(buf));

      switch (buf) {
      case command::request::set_map:
        {
          tud_vendor_read(&buf, sizeof(buf));

          mapping.count = std::min<uint8_t>(buf, count_of(mapping.items));

          gpio_put(25, true);

          auto off = 0u;
          auto len = sizeof(mapping.items);

          do {
            auto read = tud_vendor_read((uint8_t *) mapping.items + off, len);

            tud_task();

            off += read;
            len -= read;
          }
          while (len > 0);

          gpio_put(25, false);

#if defined(DEMO)
          if (!quota_remain) {
            auto ratio = (double) get_rand_64() / 0xFFFF'FFFF'FFFF'FFFF;

            const auto
              min = 90,  // 1 hour  30 minute
              max = 1'440; // 1 day

            quota_remain = ratio * (max - min) + min;
            quota_save();
          }
#endif

          break;
        }
      case command::request::set_channel_num:
        {
          tud_vendor_read(&buf, sizeof(buf));

          config.channel_num = buf & 0xF;

          break;
        }
      case command::request::set_controller_num:
        {
          tud_vendor_read(&buf, sizeof(buf));

          config.controller_num = buf & 0x7F;

          break;
        }

      case command::request::save_config:
        {
          gpio_put(25, true);
          config_save();
          gpio_put(25, false);

          break;
        }
      case command::request::get_config:
        {
          tud_vendor_write_char(command::response::get_config);

          auto off = 0u;
          auto len = sizeof(config);

          do {
            auto write = tud_vendor_write((const uint8_t *) &config + off, len);

            tud_task();

            off += write;
            len -= write;
          }
          while (len > 0);

          tud_vendor_write_flush();

          break;
        }

      case command::request::dump:
        {
          dumped = true;

          // extend deadline
          next_dumped = make_timeout_time_ms(2'000);
        }
      }
    }

    // sensor data available
    if (
#if defined(EMULATED)
      time_reached(next_out)
#else
      tm7711_read_available(pio, sensor_sm)
#endif
    ) {
#if defined(EMULATED)
      auto sensor_val = gen_samps[gen_step];

      // every 22 ms
      next_out = make_timeout_time_ms(252);
#else
      auto sensor_val = tm7711_read(pio, sensor_sm);

      // next sensor read
      tm7711_read_start(pio, sensor_sm, TM7711_READ_FAST);
#endif

      const auto [res, val] = map(mapping.items, mapping.count, sensor_val);

      if (dumped && tud_vendor_write_available() > 5) {
        tud_vendor_write_char(command::response::dump);

        tud_vendor_write(&sensor_val, sizeof(sensor_val));
        tud_vendor_write(&val, sizeof(val));

        tud_vendor_write_flush();
      }

      const bool cur_state = res == MAP_OK;

      if (prev_state || cur_state) {
#if defined(DEMO)
        if (quota_remain || get_rand_64() < 0x3000'0000'0000'0000ull)
#endif
        {
          // send Control Change MIDI packet
          uint8_t buf[3] = {
            (uint8_t) (0xB0u | config.channel_num), config.controller_num, val
          };

          tud_midi_stream_write(0, buf, sizeof(buf));
        }

#if !defined(EMULATED)
        ws2812_write(
          pio, led_sm, cur_state ? colorful<uint8_t, 0, 127>(val) : (const color_t) {0, 0, 0}
        );
#endif
      }

      prev_state = cur_state;
    }
  }
}
