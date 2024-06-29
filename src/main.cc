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
  pio_sm_put_blocking(pio, sensor_sm, 2);
#endif

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

    if (tud_ready()) {
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
                min = 60,  // 1 hour
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
        }
      }

      // sensor data available
      if (
#if defined(EMULATED)
        time_reached(next_out)
#else
        !pio_sm_is_rx_fifo_empty(pio, sensor_sm)
#endif
      ) {
#if defined(EMULATED)
        int32_t val = gen_samps[gen_step];
#else
        auto raw_val = pio_sm_get(pio, sensor_sm);

        // sign extended value
        int32_t val =
          raw_val & 0x800000 ?
            raw_val | 0xFF000000 :
            raw_val;
#endif

        auto [err, mapped_val] = map(mapping.items, mapping.count, val);

#if defined(DEMO)
        if (quota_remain || get_rand_64() < 0x3000'0000'0000'0000ull)
#endif
        {
          // send Control Change MIDI packet
          uint8_t buf[3] = {
            (uint8_t) (0xB0u | config.channel_num), config.controller_num, mapped_val
          };

          tud_midi_stream_write(0, buf, sizeof(buf));
        }

        tud_vendor_write_char(command::response::dump);
        tud_vendor_write(&val, sizeof(val));
        tud_vendor_write(&mapped_val, sizeof(mapped_val));
        tud_vendor_write_flush();

#if defined(EMULATED)
        // every 25 ms
        next_out = make_timeout_time_ms(25);
#else
        // next sensor read
        pio_sm_put_blocking(pio, sensor_sm, 2);
        pio_sm_put_blocking(
          pio, led_sm, err ? 0 : ws2812_color(
            colorful<uint8_t, 0, 127>(mapped_val)
          )
        );
#endif
      }
    }
  }
}
