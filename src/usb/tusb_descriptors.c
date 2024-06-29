#include <pico/unique_id.h>

#include "tusb.h"

typedef struct {
  const unsigned char len;
  const char *ptr;
}
lit_str_t;

static char serial[1 + PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2];

// string table
static const lit_str_t strings[] = {
#define LIT(x) {sizeof(x) - 1, x}
  // index starts from 1
  LIT("CasperX"),
  LIT("USB MIDI CC"),
  LIT(serial),
  LIT("USB MIDI CC Settings")
#undef LIT
};

static const tusb_desc_device_t desc_device = {
  .bLength = sizeof(tusb_desc_device_t),
  .bDescriptorType = TUSB_DESC_DEVICE,

  .bcdUSB = 0x0200,

  .bDeviceClass = 0x00,
  .bDeviceSubClass = 0x00,
  .bDeviceProtocol = 0x00,

  .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

  .idVendor = 0xCA52,
  .idProduct = 0x0001,
  .bcdDevice = 0x0100,

  .iManufacturer = 0x01,
  .iProduct = 0x02,
  .iSerialNumber = 0x03,

  .bNumConfigurations = 0x01
};

const uint8_t *tud_descriptor_device_cb(void) {
  return (uint8_t *) &desc_device;
}

#if TUD_OPT_HIGH_SPEED
static const tusb_desc_device_qualifier_t desc_device_qualifier = {
  .bLength = sizeof(tusb_desc_device_qualifier_t),
  .bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER,

  .bcdUSB = 0x0200,

  .bDeviceClass = 0x00,
  .bDeviceSubClass = 0x00,
  .bDeviceProtocol = 0x00,

  .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

  .bNumConfigurations = 0x01
};

const uint8_t *tud_descriptor_device_qualifier_cb(void) {
  return (uint8_t const*) &desc_device_qualifier;
}
#endif

enum {
  ITF_NUM_AUDIO_CTRL,
  ITF_NUM_AUDIO_MIDI_STREAMING,

  ITF_NUM_SETTING,

  ITF_NUM_TOTAL,
};

enum  {
  EP_NUM_MIDI = 1,
  EP_NUM_SETTING,
};

#define DESC_CFG_LEN (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN + TUD_VENDOR_DESC_LEN)

static const uint8_t fs_desc_config[] = {
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, DESC_CFG_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
  TUD_MIDI_DESCRIPTOR(ITF_NUM_AUDIO_CTRL, 2, EP_NUM_MIDI, 0x80 | EP_NUM_MIDI, 64),
  TUD_VENDOR_DESCRIPTOR(ITF_NUM_SETTING, 4, EP_NUM_SETTING, 0x80 | EP_NUM_SETTING, 64),
};

#if TUD_OPT_HIGH_SPEED
static const uint8_t hs_desc_config[] = {
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, DESC_CFG_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
  TUD_MIDI_DESCRIPTOR(ITF_NUM_AUDIO_CTRL, 2, EP_NUM_MIDI, 0x80 | EP_NUM_MIDI, 512),
  TUD_VENDOR_DESCRIPTOR(ITF_NUM_SETTING, 4, EP_NUM_SETTING, 0x80 | EP_NUM_SETTING, 512),
};

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
  (void) index;

  return tud_speed_get() == TUSB_SPEED_HIGH ?
    hs_desc_config :
    fs_desc_config;
}

const uint8_t *tud_descriptor_other_speed_configuration_cb(uint8_t index) {
  (void) index;

  static uint8_t desc_config[DESC_CFG_LEN];

  uint8_t *inv_desc_config = tud_speed_get() == TUSB_SPEED_HIGH ?
    fs_desc_config :
    hs_desc_config;

  memcpy(desc_config, inv_desc_config, DESC_CFG_LEN);

  desc_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

  return desc_config;
}
#else
const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
  (void) index;

  return fs_desc_config;
}
#endif

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t lang_id) {
  (void) lang_id;

  // load unique board ID once
  if (!*serial) pico_get_unique_board_id_string(serial, sizeof(serial));

  if (count_of(strings) < index) return NULL;

  static union {
    tusb_desc_string_t desc;
    uint8_t buf[66];
  }
  shared;

  tusb_desc_string_t *desc = &shared.desc;

  desc->bLength = sizeof(tusb_desc_string_t);
  desc->bDescriptorType = TUSB_DESC_STRING;

  if (index == 0) {
    // English language only
    const int16_t lang_id = 0x0409;

    desc->bLength += 2;

    memcpy(desc->unicode_string, &lang_id, 2);
  }
  else {
    const lit_str_t *string = &strings[index - 1];

    uint8_t len = string->len;

    // limit length
    if (len > 32) len = 32;

    desc->bLength += 2 * len;

    // Convert ASCII string into UTF-16
    for (uint8_t i = 0; i < len; ++i) desc->unicode_string[i] = string->ptr[i];
  }

  return (uint16_t *) shared.buf;
}
